/*
** Example: type metadata
**
** This example is full of commentary on strategies for setting up Metadesk
** programs. There are many many ways a system like this could be built,
** this example only demonstrates one way. Everything here should be taken as
** our best attempt to pass on insights about using Metadesk, and not as the
** "one true way".
**
** This example shows what a typical Metadesk based metaprogram for generating
** type metadata looks like. Metadesk can be used for all sorts of other
** purposes, managing data tables, authoring content, configuration systems,
** and more, but a very common reason that a C programmer reaches for a
** metaprogramming system is to make up for C's lack of type metadata.
**
** The "Metadesk way" of solving this problem is to move the hand written type
** definitions into metadesk files, and to generate the C structs AND the 
** tables for type metadata from the metaprogram.
**
** A more common approach is to mark up C types and create a custom parser
** for the modified C hybdrid. That approach requires a lot of heuristics to
** deal with the complex grammar of C code, and is especially hard to make
** both robust and reusable. The Metadesk way makes the problem a lot more
** simple, if you are comfortable with generating C types instead of writing
** them by hand in C.
**
**
** Files:
**  type_metadata.h - This example is big enough that we pulled out the types
**                    and function declarations from the metaprogram to keep
**                    things organized.
**  type_info.h     - This header is included in the "final program" and
**                    defines the types that will form the tables of metdata.
**  type_info_final_program.c - In addition to the metaprogram this example
**                    includes a "final program", i.e. one that depends on
**                    generated code. There is more commentary there.
**  generated/ *    - This is the output folder where the generated files
**                    should be saved by the metaprogram.
**  types.mdesk     - Sample input for this metaprogram. The code generated
**                    from this metadesk file will be necessary to make
**                    type_info_final_program.c compile.
**  bad_types.mdesk - Sample input for seeing some of the error checking work.
**
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "type_metadata.h"

#include "md.c"

#include <filesystem>

//static MD_Arena *arena = 0;

static FILE *error_file = 0;

// node maps

// @notes As we analyze the Metadesk tree we create more data. In this example
//  the new data is stored in two major types GEN_TypeInfo and GEN_MapInfo.
//  We form a list for each type of these "processed" types holds a pointer
//  back to the original MD_Node that generated it, and room for information
//  that will be equiped to the "processed" type in later stages of analysis.
//  We also use the MD_Map helper to create a string -> pointer mapping so that
//  we can look up the "processed" info pointers by name after the initial
//  gather stage.


struct GEN_FileData
{
    MD_Arena* arena = 0;
    GEN_TypeInfo *first_type = 0;
    GEN_TypeInfo *last_type = 0;
    MD_Map type_map = MD_ZERO_STRUCT;

    GEN_MapInfo *first_map = 0;
    GEN_MapInfo *last_map = 0;
    MD_Map map_map = MD_ZERO_STRUCT;

    MD_String8List include_list = MD_ZERO_STRUCT;
    MD_String8 filename = MD_ZERO_STRUCT;

    GEN_FileData* include_filedata = 0;
};


//~ helpers ///////////////////////////////////////////////////////////////////

MD_Node*
gen_get_child_value(MD_Node *parent, MD_String8 child_name)
{
    MD_Node *child = MD_ChildFromString(parent, child_name, 0);
    MD_Node *result = child->first_child;
    return(result);
}

GEN_TypeInfo*
gen_resolve_type_info_from_string(MD_String8 name, GEN_FileData* filedata)
{
    GEN_TypeInfo *result = 0;
    // @notes The MD_Map helper is a "flexibly" typed hash table. It's keys can
    //  be a mix of strings and pointers. Here MD_MapKeyStr(name) is making the
    //  `name` string into a key for the map. The lookup function returns a
    //  "map slot" because the map is not restricted to storing just one value
    //  per key, if we were using it that way we could use MD_MapScan to
    //  iterate through the map slots.
    MD_MapSlot *slot = MD_MapLookup(&filedata->type_map, MD_MapKeyStr(name));
    if (slot != 0)
    {
        result = (GEN_TypeInfo*)slot->val;
    }
    else if (filedata->include_filedata)
    {
        result = gen_resolve_type_info_from_string(name, filedata->include_filedata);
    }
    return(result);
}

GEN_TypeInfo*
gen_resolve_type_info_from_referencer(MD_Node *reference, GEN_FileData* filedata)
{
    GEN_TypeInfo *result = gen_resolve_type_info_from_string(reference->string, filedata);
    return(result);
}

GEN_TypeEnumerant*
gen_enumerant_from_name(GEN_TypeInfo *enum_type, MD_String8 name)
{
    GEN_TypeEnumerant *result = 0;
    for (GEN_TypeEnumerant *enumerant = enum_type->first_enumerant;
         enumerant != 0;
         enumerant = enumerant->next)
    {
        if (MD_S8Match(name, enumerant->node->string, 0))
        {
            result = enumerant;
            break;
        }
    }
    return(result);
}

GEN_MapCase*
gen_map_case_from_enumerant(GEN_MapInfo *map, GEN_TypeEnumerant *enumerant)
{
    GEN_MapCase *result = 0;
    for (GEN_MapCase *map_case = map->first_case;
         map_case != 0;
         map_case = map_case->next)
    {
        if (map_case->in_enumerant == enumerant)
        {
            result = map_case;
            break;
        }
    }
    return(result);
}

MD_Node*
gen_get_symbol_md_node_by_name(MD_String8 name, GEN_FileData* filedata)
{
    MD_Node *result = MD_NilNode();
    MD_MapSlot *type_slot = MD_MapLookup(&filedata->type_map, MD_MapKeyStr(name));
    if (type_slot != 0)
    {
        GEN_TypeInfo *type_info = (GEN_TypeInfo*)type_slot->val;
        result = type_info->node;
    }
    MD_MapSlot *map_slot = MD_MapLookup(&filedata->map_map, MD_MapKeyStr(name));
    if (map_slot != 0)
    {
        GEN_MapInfo *map_info = (GEN_MapInfo*)map_slot->val;
        result = map_info->node;
    }
    if (MD_NodeIsNil(result) && filedata->include_filedata)
    {
        result = gen_get_symbol_md_node_by_name(name, filedata->include_filedata);
    }
    return(result);
}

MD_String8 remove_file_extension(MD_String8 full_filename)
{
    // get non-extension filename
    MD_u8* str_end = full_filename.str + full_filename.size-1;
    MD_u8* ext_pos = str_end;
    int extension_length = 1;
    for (; *--str_end != '.' ; extension_length++) // reverse walk to '.'
    {}
    extension_length++;
    MD_String8 filename_noext = MD_S8(full_filename.str, full_filename.size - extension_length);
    return filename_noext;
}

void
gen_type_resolve_error(MD_Node *reference)
{
    MD_CodeLoc loc = MD_CodeLocFromNode(reference);
    MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                       "could not resolve type name '%.*s'", MD_S8VArg(reference->string));
}

void
gen_duplicate_symbol_error(MD_Node *new_node, MD_Node *existing_node)
{
    MD_CodeLoc loc = MD_CodeLocFromNode(new_node);
    MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                       "Symbol name '%.*s' is already used",
                       MD_S8VArg(new_node->string));
    MD_CodeLoc existing_loc = MD_CodeLocFromNode(existing_node); 
    MD_PrintMessageFmt(error_file, existing_loc, MD_MessageKind_Note,
                       "See '%.*s' is already used",
                       MD_S8VArg(existing_node->string));
}

void
gen_check_and_do_duplicate_symbol_error(MD_Node *new_node, GEN_FileData* filedata)
{
    MD_Node *existing = gen_get_symbol_md_node_by_name(new_node->string, filedata);
    if (!MD_NodeIsNil(existing))
    {
        gen_duplicate_symbol_error(new_node, existing);
    }
    else if (filedata->include_filedata)
    {
        gen_check_and_do_duplicate_symbol_error(new_node, filedata->include_filedata);
    }
}


//~ analyzers /////////////////////////////////////////////////////////////////

// @notes The first stage of processing is to loop over the top level nodes
//  from each parse. We are using the tags `@type` and `@map` to mark the nodes
//  that this generator will process. Whenever we see one of those tags we
//  create a GEN_TypeInfo or GEN_MapInfo to gather up information from the
//  stages of analysis, and we insert the new info pointer into the appropriate
//  map. On the types we do a little bit of the analysis right in this function
//  to figure out which "type kind" it is, this lets us avoid ever having info
//  where the kind field is not one of the expected values.

void
gen_gather_types_and_maps(GEN_FileData* filedata, MD_Node* root)
{
    // for each parsed FILE passed in
    //for(MD_EachNode(ref, list->first_child))
    //{
        // topmost root node of a parsed file
        //MD_Node *root = MD_ResolveNodeFromReference(ref);
        for(MD_EachNode(node, root->first_child))
        {
            // gather type
            MD_Node *type_tag =  MD_TagFromString(node, MD_S8Lit("type"), 0);
            
            // if we are a @type
            if (!MD_NodeIsNil(type_tag))
            {
                gen_check_and_do_duplicate_symbol_error(node, filedata);
                
                GEN_TypeKind kind = GEN_TypeKind_Null;
                MD_Node   *tag_arg_node = type_tag->first_child;
                MD_String8 tag_arg_str = tag_arg_node->string;
                if (MD_S8Match(tag_arg_str, MD_S8Lit("basic"), 0))
                {
                    kind = GEN_TypeKind_Basic;
                }
                else if (MD_S8Match(tag_arg_str, MD_S8Lit("struct"), 0))
                {
                    kind = GEN_TypeKind_Struct;
                }
                else if (MD_S8Match(tag_arg_str, MD_S8Lit("enum"), 0))
                {
                    kind = GEN_TypeKind_Enum;
                }
                
                if (kind == GEN_TypeKind_Null)
                {
                    MD_CodeLoc loc = MD_CodeLocFromNode(node);
                    MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                                       "Unrecognized type kind '%.*s'",
                                       MD_S8VArg(tag_arg_str));
                }
                else
                {
                    GEN_TypeInfo *type_info = MD_PushArrayZero(filedata->arena, GEN_TypeInfo, 1);
                    type_info->kind = kind;
                    type_info->node = node;
                    MD_QueuePush(filedata->first_type, filedata->last_type, type_info);
                    MD_MapInsert(filedata->arena, &filedata->type_map, MD_MapKeyStr(node->string), type_info);
                }
            }
            
            // if we are an @map
            if (MD_NodeHasTag(node, MD_S8Lit("map"), 0))
            {
                gen_check_and_do_duplicate_symbol_error(node, filedata);
                
                GEN_MapInfo *map_info = MD_PushArrayZero(filedata->arena, GEN_MapInfo, 1);
                map_info->node = node;
                MD_QueuePush(filedata->first_map, filedata->last_map, map_info);
                MD_MapInsert(filedata->arena, &filedata->map_map, MD_MapKeyStr(node->string), map_info);
            }
        }
    //}
}

void
gen_parse_includes(GEN_FileData* filedata, MD_Node* root)
{
    // "preprocessor"
    for(MD_EachNode(node, root->first_child))
    {
        if (MD_NodeHasTag(node, MD_S8Lit("include"), 0))
        {
            MD_Node* include_tag = node->first_tag; // include(...)
            MD_Node* include_path_node = include_tag->first_child;
            if (!MD_NodeIsNil(include_path_node))
            {
                MD_String8 include_path = include_path_node->string;
                MD_S8ListPush(filedata->arena, &filedata->include_list, include_path);
            }
        }
    }
}

void
gen_check_duplicate_member_names(GEN_FileData* filedata)
{
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        MD_Node *type_root_node = type->node;
        for (MD_EachNode(member_node, type_root_node->first_child))
        {
            MD_String8 name = member_node->string;
            for (MD_EachNode(check_node, type_root_node->first_child))
            {
                if (member_node == check_node)
                {
                    break;
                }
                if (MD_S8Match(name, check_node->string, 0))
                {
                    MD_CodeLoc my_loc = MD_CodeLocFromNode(member_node);
                    MD_CodeLoc og_loc = MD_CodeLocFromNode(check_node);
                    MD_PrintMessageFmt(error_file, my_loc, MD_MessageKind_Error,
                                       "'%.*s' is already defined", MD_S8VArg(name));
                    MD_PrintMessageFmt(error_file, og_loc, MD_MessageKind_Note,
                                       "see previous definition of '%.*s'", MD_S8VArg(name));
                    break;
                }
            }
        }
    }
}

// @notes In the next few stages of analysis we 'equip' the info nodes we 
//  gathered with further information by examining the sub-trees rooted at the 
//  metadesk nodes we saw durring the gather phase.

void
gen_equip_basic_type_size(GEN_FileData* filedata)
{
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == GEN_TypeKind_Basic)
        {
            // extract the size
            int size = 0;
            
            // this size_node is the size label on basic types
            // @type(basic) u32: 4;   <- that 4 is the size_node
            MD_Node *size_node = type->node->first_child;
            MD_Node *error_at = 0;
            if (MD_NodeIsNil(size_node))
            {
                error_at = type->node;
            }
            else
            {
                MD_String8 size_string = size_node->string;
                if (!MD_StringIsCStyleInt(size_string))
                {
                    error_at = size_node;
                }
                else
                {
                    size = (int)MD_CStyleIntFromString(size_string);
                }
            }
            if (error_at != 0)
            {
                MD_CodeLoc loc = MD_CodeLocFromNode(error_at);
                MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                MD_S8Lit("a basic type requires a plain integer size specifier"));
            }
            
            // save the size
            type->size = size;
        }
    }
}

void
gen_equip_struct_members(GEN_FileData* filedata)
{
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == GEN_TypeKind_Struct)
        {
            // build the list
            MD_b32 got_list = 1;
            GEN_TypeMember *first_member = 0;
            GEN_TypeMember *last_member = 0;
            int member_count = 0;
            
            // this is the node for the "name" of the struct (like "Vector2")
            MD_Node *type_root_node = type->node;
            for (MD_EachNode(member_node, type_root_node->first_child))
            {
                // the "x" or "y" in Vector2
                MD_Node *type_name_node = member_node->first_child;
                
                // missing type node?
                if (MD_NodeIsNil(type_name_node))
                {
                    MD_CodeLoc loc = MD_CodeLocFromNode(member_node);
                    MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                    MD_S8Lit("Missing type name for member"));
                    got_list = 0;
                    goto skip_member;
                }
                
                // has type node:

                // the "x" or "y" in Vector2
                MD_String8 type_name = type_name_node->string;
                // fetches type info from the type_map from the type's name
                GEN_TypeInfo *type_info = gen_resolve_type_info_from_string(type_name, filedata);
                
                // could not resolve type?
                if (type_info == 0)
                {
                    gen_type_resolve_error(type_name_node);
                    got_list = 0;
                    goto skip_member;
                }
                
                // resolved type:
                if (got_list)
                {
                    GEN_TypeMember *array_count = 0;
                    MD_Node *array_tag = MD_TagFromString(type_name_node, MD_S8Lit("array"), 0);
                    // if we're an array struct member
                    if (!MD_NodeIsNil(array_tag))
                    {
                        MD_Node *array_count_referencer = array_tag->first_child;
                        if (array_count_referencer->string.size == 0)
                        {
                            MD_CodeLoc loc = MD_CodeLocFromNode(array_tag);
                            MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                            MD_S8Lit("array tags must specify a parameter for their count"));
                        }
                        else
                        {
                            // if we have a array count referencer, it must be a member of that same struct
                            MD_Node *array_count_member_node =
                                MD_ChildFromString(type_root_node, array_count_referencer->string, 0);
                            if (MD_NodeIsNil(array_count_member_node))
                            {
                                MD_CodeLoc loc = MD_CodeLocFromNode(array_count_referencer);
                                MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                                                   "'%.*s' is not a member of %.*s",
                                                   MD_S8VArg(array_count_referencer->string), MD_S8VArg(type_name));
                            }
                            else
                            {
                                for (GEN_TypeMember *member_it = first_member;
                                     member_it != 0;
                                     member_it = member_it->next)
                                {
                                    if (member_it->node == array_count_member_node)
                                    {
                                        array_count = member_it;
                                        break;
                                    }
                                }
                                if (array_count == 0)
                                {
                                    // ensuring the reference count variable for an array is declared before
                                    // the array declaration
                                    MD_CodeLoc loc = MD_CodeLocFromNode(array_count_referencer);
                                    MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                                                       "'%.*s' comes after this array",
                                                       MD_S8VArg(array_count_referencer->string), MD_S8VArg(type_name));
                                }
                            }
                        }
                    }
                    
                    GEN_TypeMember *member = MD_PushArray(filedata->arena, GEN_TypeMember, 1);
                    member->node = member_node;
                    member->type = type_info;
                    member->array_count = array_count;
                    member->member_index = member_count;
                    MD_QueuePush(first_member, last_member, member);
                    member_count += 1;
                }
                
                skip_member:;
            }
            
            // save the list
            if (got_list)
            {
                type->first_member = first_member;
                type->last_member = last_member;
                type->member_count = member_count;
            }
        }
    }
}

void
gen_equip_enum_underlying_type(GEN_FileData* filedata)
{
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == GEN_TypeKind_Enum)
        {
            // extract underlying type
            GEN_TypeInfo *underlying_type = 0;
            
            MD_Node *type_node = type->node;
            MD_Node *type_tag = MD_TagFromString(type_node, MD_S8Lit("type"), 0);
            MD_Node *type_tag_param = type_tag->first_child;
            MD_Node *underlying_type_ref = type_tag_param->first_child;
            if (!MD_NodeIsNil(underlying_type_ref))
            {
                GEN_TypeInfo *resolved_type = gen_resolve_type_info_from_referencer(underlying_type_ref, filedata);
                if (resolved_type == 0)
                {
                    gen_type_resolve_error(underlying_type_ref);
                }
                else
                {
                    if (resolved_type->kind != GEN_TypeKind_Basic)
                    {
                        MD_CodeLoc loc = MD_CodeLocFromNode(underlying_type_ref);
                        MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                                           "'%.*s' is not a basic type",
                                           MD_S8VArg(underlying_type_ref->string));
                    }
                    else
                    {
                        underlying_type = resolved_type;
                    }
                }
            }
            
            // save underlying type
            type->underlying_type = underlying_type;
        }
    }
}

void
gen_equip_enum_members(GEN_FileData* filedata)
{
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == GEN_TypeKind_Enum)
        {
            // build the list
            MD_b32 got_list = 1;
            GEN_TypeEnumerant *first_enumerant = 0;
            GEN_TypeEnumerant *last_enumerant = 0;
            int enumerant_count = 0;
            
            int next_implicit_value = 0;
            
            // actual enum
            MD_Node *type_root_node = type->node;
            for (MD_EachNode(enumerant_node, type_root_node->first_child))
            {
                // each value of enum
                MD_Node *value_node = enumerant_node->first_child;
                int value = 0;
                
                // missing value node?
                if (MD_NodeIsNil(value_node))
                {
                    value = next_implicit_value;
                    next_implicit_value += 1;
                }
                
                // has value node
                else
                {
                    MD_String8 value_string = value_node->string;
                    if (!MD_StringIsCStyleInt(value_string))
                    {
                        got_list = 0;
                        goto skip_enumerant;
                    }
                    value = (int)MD_CStyleIntFromString(value_string);
                }
                
                // set next implicit value
                next_implicit_value = value + 1;
                
                // save enumerant
                if (got_list)
                {
                    GEN_TypeEnumerant *enumerant = MD_PushArray(filedata->arena, GEN_TypeEnumerant, 1);
                    enumerant->node = enumerant_node;
                    enumerant->value = value;
                    MD_QueuePush(first_enumerant, last_enumerant, enumerant);
                    enumerant_count += 1;
                }
                
                skip_enumerant:;
            }
            
            // save the list
            if (got_list)
            {
                type->first_enumerant = first_enumerant;
                type->last_enumerant = last_enumerant;
                type->enumerant_count = enumerant_count;
            }
        }
    }
}

void
gen_equip_map_in_out_types(GEN_FileData* filedata)
{
    for (GEN_MapInfo *map = filedata->first_map;
         map != 0;
         map = map->next)
    {
        MD_Node *map_root_node = map->node;
        MD_Node *map_tag = MD_TagFromString(map_root_node, MD_S8Lit("map"), 0);
        
        // NOTE we could use an expression parser here to make this fancier
        // and check for the 'In -> Out' semicolon delimited syntax more
        // carefully, this isn't checking it very rigorously. But there are
        // no other cases we need to expect so far so being a bit sloppy
        // buys us a lot of simplicity.
        MD_Node *in_node = map_tag->first_child;
        MD_Node *arrow = in_node->next;
        MD_Node *out_node = arrow->next;
        {
            MD_Node *error_at = 0;
            if (MD_NodeIsNil(in_node))
            {
                error_at = map_tag;
            }
            else if (!MD_S8Match(arrow->string, MD_S8Lit("->"), 0) ||
                     MD_NodeIsNil(out_node))
            {
                error_at = in_node;
            }
            if (error_at != 0)
            {
                MD_CodeLoc loc = MD_CodeLocFromNode(error_at);
                MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                MD_S8Lit("a map's type should be specified like: `In -> Out`"));
            }
        }
        
        // construct type info for map
        GEN_TypedMapInfo *typed_map = 0;
        {
            // resolve in type info
            GEN_TypeInfo *in_type_info = gen_resolve_type_info_from_referencer(in_node, filedata);
            if (in_type_info != 0 &&
                in_type_info->kind != GEN_TypeKind_Enum)
            {
                MD_CodeLoc loc = MD_CodeLocFromNode(in_node);
                MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                MD_S8Lit("a map's In type should be an enum"));
                in_type_info = 0;
            }
            
            // resolve out type info
            GEN_TypeInfo *out_type_info = gen_resolve_type_info_from_referencer(out_node, filedata);
            int out_is_type_info_ptr = 0;
            if (out_type_info == 0)
            {
                MD_String8 out_name = out_node->string;
                if (MD_S8Match(out_name, MD_S8Lit("$Type"), 0))
                {
                    out_is_type_info_ptr = 1;
                }
                else
                {
                    gen_type_resolve_error(out_node);
                }
            }
            
            // assemble typed map
            if (in_type_info != 0 && (out_type_info != 0 || out_is_type_info_ptr))
            {
                typed_map = MD_PushArray(filedata->arena, GEN_TypedMapInfo, 1);
                
                // fill primary values
                typed_map->in = in_type_info;
                typed_map->out = out_type_info;
                typed_map->out_is_type_info_ptr = out_is_type_info_ptr;
                
                // fill derived values
                typed_map->in_type_string  = in_type_info->node->string;
                if (out_is_type_info_ptr)
                {
                    typed_map->out_type_string = MD_S8Lit("TypeInfo*");
                }
                else
                {
                    typed_map->out_type_string = out_type_info->node->string;
                }
            }
        }
        
        // check for named children in the map tag
        int is_complete = MD_NodeHasChild(map_tag, MD_S8Lit("complete"), 0);
        MD_Node *default_val = gen_get_child_value(map_tag, MD_S8Lit("default"));
        MD_Node *auto_val = gen_get_child_value(map_tag, MD_S8Lit("auto"));
        
        // save to map
        map->typed_map = typed_map;
        map->is_complete = is_complete;
        map->default_val = default_val;
        map->auto_val = auto_val;
    }
}

void
gen_equip_map_cases(GEN_FileData* filedata)
{
    for (GEN_MapInfo *map = filedata->first_map;
         map != 0;
         map = map->next)
    {
        GEN_TypedMapInfo *typed_map = map->typed_map;
        if (typed_map != 0)
        {
            
            // get in type
            GEN_TypeInfo *in_type = typed_map->in;
            
            // build the list
            MD_b32 got_list = 1;
            GEN_MapCase *first_case = 0;
            GEN_MapCase *last_case = 0;
            int case_count = 0;
            
            MD_Node *map_root_node = map->node;
            
            for (MD_Node *case_node = map_root_node->first_child;
                 !MD_NodeIsNil(case_node);
                 case_node = MD_FirstNodeWithFlags(case_node->next, MD_NodeFlag_IsAfterComma))
            {
                // extract in & out
                MD_Node *in = case_node;
                MD_Node *arrow = in->next;
                MD_Node *out = arrow->next;
                if (!MD_S8Match(arrow->string, MD_S8Lit("->"), 0) ||
                    MD_NodeIsNil(out))
                {
                    MD_CodeLoc loc = MD_CodeLocFromNode(in);
                    MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                                    MD_S8Lit("a map's case should be specified like: `in -> out,`"));
                    got_list = 0;
                    goto skip_case;
                }
                
                // get enumerant from in_type
                GEN_TypeEnumerant *in_enumerant = gen_enumerant_from_name(in_type, in->string);
                if (in_enumerant == 0)
                {
                    MD_CodeLoc loc = MD_CodeLocFromNode(in);
                    MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                                       "'%.*s' is not a value in the enum '%.*s'",
                                       MD_S8VArg(in->string), MD_S8VArg(in_type->node->string));
                    got_list = 0;
                    goto skip_case;
                }
                
                // save case
                if (got_list)
                {
                    GEN_MapCase *map_case = MD_PushArray(filedata->arena, GEN_MapCase, 1);
                    map_case->in_enumerant = in_enumerant;
                    map_case->out = out;
                    MD_QueuePush(first_case, last_case, map_case);
                    case_count += 1;
                }
                
                skip_case:;
            }
            
            // save the list
            if (got_list)
            {
                map->first_case = first_case;
                map->last_case = last_case;
                map->case_count = case_count;
            }
        }
    }
}

void
gen_check_duplicate_cases(GEN_FileData* filedata)
{
    for (GEN_MapInfo *map = filedata->first_map;
         map != 0;
         map = map->next)
    {
        
        for (GEN_MapCase *node = map->first_case;
             node != 0;
             node = node->next)
        {
            GEN_TypeEnumerant *enumerant = node->in_enumerant;
            MD_String8 name = enumerant->node->string;
            for (GEN_MapCase *check = map->first_case;
                 check != 0;
                 check = check->next)
            {
                if (node == check)
                {
                    break;
                }
                if (enumerant == check->in_enumerant)
                {
                    MD_CodeLoc my_loc = MD_CodeLocFromNode(enumerant->node);
                    MD_CodeLoc og_loc = MD_CodeLocFromNode(check->in_enumerant->node);
                    MD_PrintMessageFmt(error_file, my_loc, MD_MessageKind_Error,
                                       "'%.*s' is already defined", MD_S8VArg(name));
                    MD_PrintMessageFmt(error_file, og_loc, MD_MessageKind_Note,
                                       "see previous definition of '%.*s'", MD_S8VArg(name));
                    break;
                }
                if (enumerant->value == check->in_enumerant->value)
                {
                    MD_CodeLoc my_loc = MD_CodeLocFromNode(enumerant->node);
                    MD_CodeLoc og_loc = MD_CodeLocFromNode(check->in_enumerant->node);
                    MD_PrintMessageFmt(error_file, my_loc, MD_MessageKind_Error,
                                       "'%.*s' has value %d which is already defined",
                                       MD_S8VArg(name), enumerant->value);
                    MD_PrintMessageFmt(error_file, og_loc, MD_MessageKind_Note,
                                       "see previous definition '%.*s'",
                                       MD_S8VArg(check->in_enumerant->node->string));
                    break;
                }
            }
        }
        
    }
}

void
gen_check_complete_map_cases(GEN_FileData* filedata)
{
    for (GEN_MapInfo *map = filedata->first_map;
         map != 0;
         map = map->next)
    {
        GEN_TypedMapInfo *typed_map = map->typed_map;
        if (typed_map != 0 && map->is_complete)
        {
            int printed_message_for_this_map = 0;
            
            GEN_TypeInfo *in_type = typed_map->in;
            for (GEN_TypeEnumerant *enumerant = in_type->first_enumerant;
                 enumerant != 0;
                 enumerant = enumerant->next)
            {
                GEN_MapCase *existing_case = gen_map_case_from_enumerant(map, enumerant);
                
                if (existing_case == 0)
                {
                    if (!printed_message_for_this_map)
                    {
                        printed_message_for_this_map = 1;
                        MD_CodeLoc map_loc = MD_CodeLocFromNode(map->node); 
                        MD_PrintMessage(error_file, map_loc, MD_MessageKind_Warning,
                                        MD_S8Lit("map marked as complete is missing a case (or more)"));
                    }
                    MD_String8 enumerant_name = enumerant->node->string;
                    MD_CodeLoc enumerant_loc = MD_CodeLocFromNode(enumerant->node);
                    MD_PrintMessageFmt(error_file, enumerant_loc, MD_MessageKind_Note,
                                       "see enumerant '%.*s'", MD_S8VArg(enumerant_name));
                }
            }
            
        }
    }
}

//~ generators ////////////////////////////////////////////////////////////////

// @notes Each generator function handles generating every instance of a
//  particular function. This means that there is only one place where the
//  generator gets called and only one place where the generated code gets
//  written. This keeps things simple but may be quite limiting depending on
//  the use case.
//
//  For instance, to have multiple groups of types and metadata generated to
//  different .h/.c file pairs, we could make the output file name a parameter
//  on the command line, and run the generator multiple times with different
//  metadesk files.
//
//  If we wanted to support references to entities accross files we would have
//  to handle all the files in one run of the generator, and the generator
//  functions would need to be called with different 'entities' for different
//  output files.

void
gen_output_include_statements(FILE* out, GEN_FileData* filedata)
{
    MD_PrintGenNoteCComment(out);
    
    for (MD_String8Node* include = filedata->include_list.first; 
        include != 0; 
        include = include->next)
    {
        MD_String8 include_string = include->string;
        MD_String8 include_string_noext = remove_file_extension(include_string);
        fprintf(out, "#include \"%.*s.h\"\n", MD_S8VArg(include_string_noext));
    }
}

void
gen_type_definitions_from_types(FILE *out, GEN_FileData* filedata)
{
    // @notes This Metadesk helper generates a comment that points back here.
    //  Generating a comment like this can help a lot to with issues later.
    MD_PrintGenNoteCComment(out);
    
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        switch (type->kind)
        {
            default:break;
            
            case GEN_TypeKind_Struct:
            {
                MD_String8 struct_name = type->node->string;
                // don't need typedef struct in c++ 
                //fprintf(out, "typedef struct %.*s %.*s;\n",
                        //MD_S8VArg(struct_name), MD_S8VArg(struct_name));
                fprintf(out, "struct %.*s\n", MD_S8VArg(struct_name));
                fprintf(out, "{\n");
                for (GEN_TypeMember *member = type->first_member;
                     member != 0;
                     member = member->next)
                {
                    MD_String8 type_name = member->type->node->string;
                    MD_String8 member_name = member->node->string;
                    if (member->array_count != 0)
                    {
                        fprintf(out, "\t%.*s *%.*s;\n", MD_S8VArg(type_name), MD_S8VArg(member_name));
                    }
                    else
                    {
                        fprintf(out, "\t%.*s %.*s;\n", MD_S8VArg(type_name), MD_S8VArg(member_name));
                    }
                }
                fprintf(out, "};\n");
            } break;
            
            case GEN_TypeKind_Enum:
            {
                MD_String8 enum_name = type->node->string;
                GEN_TypeInfo *underlying_type = type->underlying_type;
                
                // cpp style enum
                MD_String8 underlying_type_name = MD_S8Lit("int");
                if (underlying_type != 0)
                {
                    underlying_type_name = underlying_type->node->string;
                }
                fprintf(out, "enum %.*s : %.*s\n", MD_S8VArg(enum_name), MD_S8VArg(underlying_type_name));
                fprintf(out, "{\n");
                
                // enum body
                for (GEN_TypeEnumerant *enumerant = type->first_enumerant;
                     enumerant != 0;
                     enumerant = enumerant->next)
                {
                    MD_String8 member_name = enumerant->node->string;
                    fprintf(out, "\t%.*s = %d,\n", MD_S8VArg(member_name), enumerant->value);
                }
                
                // enum footer
                fprintf(out, "};\n");
                
            } break;
            case GEN_TypeKind_Basic:
            {
                MD_Node* basic_type_basic = type->node->first_tag->first_child;
                // if we have tags after "basic", treat it as a typedef
                if (MD_S8Match(basic_type_basic->string, MD_S8Lit("basic"), 0) && !MD_NodeIsNil(basic_type_basic->next))
                {
                    // specifying a tag after "basic" on an @type specifies the underlying typedef
                    MD_Node* typedef_tag = basic_type_basic->next;
                    MD_String8 typedef_underlying_type_name = { typedef_tag->string.str, 0 };
                    // things like "unsigned int" will not show up in the same string. 
                    // Need to traverse ->next to get names with more than one word
                    int are_spaces_in_type = 0;
                    for (MD_EachNode(type_word, typedef_tag))
                    {
                        typedef_underlying_type_name.size += type_word->string.size + 1; // + 1 for space
                        are_spaces_in_type = 1;
                    }
                    // if there were spaces in our type name, our size is 1 too big
                    if (are_spaces_in_type)
                    {
                        typedef_underlying_type_name.size--;
                    }
                    MD_String8 typedef_alias_type_name = type->node->string;
                    fprintf(out, "typedef %.*s %.*s;\n", MD_S8VArg(typedef_underlying_type_name), MD_S8VArg(typedef_alias_type_name));
                }
            } break;
        }
        fprintf(out, "\n");
    }
    
    fprintf(out, "\n");
}

void
gen_function_declarations_from_maps(FILE *out, GEN_FileData* filedata)
{
    MD_PrintGenNoteCComment(out);
    
    for (GEN_MapInfo *map = filedata->first_map;
         map != 0;
         map = map->next)
    {
        GEN_TypedMapInfo *typed_map = map->typed_map;
        if (typed_map != 0)
        {
            MD_String8 in_type  = typed_map->in_type_string;
            MD_String8 out_type = typed_map->out_type_string;
            
            fprintf(out, "%.*s %.*s(%.*s v);\n",
                    MD_S8VArg(out_type), MD_S8VArg(map->node->string), MD_S8VArg(in_type));
        }
    }
    
    fprintf(out, "\n");
}

void
gen_type_info_declarations_from_types(FILE *out, GEN_FileData* filedata)
{
    MD_PrintGenNoteCComment(out);
    
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        MD_String8 name = type->node->string;
        fprintf(out, "extern TypeInfo %.*s_type_info;\n", MD_S8VArg(name));
    }
    
    fprintf(out, "\n");
}

void
gen_struct_member_tables_from_types(FILE *out, GEN_FileData* filedata)
{
    MD_PrintGenNoteCComment(out);
    
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == GEN_TypeKind_Struct)
        {
            MD_String8 type_name = type->node->string;
            int member_count = type->member_count;
            
            fprintf(out, "TypeInfoMember %.*s_members[%d] = {\n", MD_S8VArg(type_name), member_count);
            
            for (GEN_TypeMember *member = type->first_member;
                 member != 0;
                 member = member->next)
            {
                MD_String8 member_name = member->node->string;
                MD_String8 member_type_name = member->type->node->string;
                int array_count_member_index = -1;
                if (member->array_count != 0)
                {
                    array_count_member_index = member->array_count->member_index;
                }
                fprintf(out, "\t{\"%.*s\", %d, %d, &%.*s_type_info},\n",
                        MD_S8VArg(member_name), (int)member_name.size,
                        array_count_member_index, MD_S8VArg(member_type_name));
            }
            
            fprintf(out, "};\n");
        }
    }
    
    fprintf(out, "\n");
}

void
gen_enum_member_tables_from_types(FILE *out, GEN_FileData* filedata)
{
    MD_PrintGenNoteCComment(out);
    
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        if (type->kind == GEN_TypeKind_Enum)
        {
            MD_String8 type_name = type->node->string;
            int enumerant_count = type->enumerant_count;
            
            fprintf(out, "TypeInfoEnumerant %.*s_members[%d] = {\n",
                    MD_S8VArg(type_name), enumerant_count);
            for (GEN_TypeEnumerant *enumerant = type->first_enumerant;
                 enumerant != 0;
                 enumerant = enumerant->next)
            {
                MD_String8 enumerant_name = enumerant->node->string;
                int value = enumerant->value;
                fprintf(out, "\t{\"%.*s\", %d, %d},\n",
                        MD_S8VArg(enumerant_name), (int)enumerant_name.size,
                        value);
            }
            
            fprintf(out, "};\n");
        }
    }
    
    fprintf(out, "\n");
}

void
gen_type_info_definitions_from_types(FILE *out, GEN_FileData* filedata)
{
    MD_ArenaTemp scratch = MD_GetScratch(0, 0);
    
    MD_PrintGenNoteCComment(out);
    
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        MD_String8 type_name = type->node->string;
        
        switch (type->kind)
        {
            default:break;
            
            case GEN_TypeKind_Basic:
            {
                int size = type->size;
                fprintf(out, "TypeInfo %.*s_type_info = "
                        "{TypeKind_Basic, \"%.*s\", %d, %d, 0, 0};\n",
                        MD_S8VArg(type_name),
                        MD_S8VArg(type_name), (int)type_name.size, size);
            }break;
            
            case GEN_TypeKind_Struct:
            {
                int child_count = type->member_count;
                fprintf(out, "TypeInfo %.*s_type_info = "
                        "{TypeKind_Struct, \"%.*s\", %d, %d, %.*s_members, 0};\n",
                        MD_S8VArg(type_name),
                        MD_S8VArg(type_name), (int)type_name.size, child_count, MD_S8VArg(type_name));
            }break;
            
            case GEN_TypeKind_Enum:
            {
                MD_String8 underlying_type_ptr_expression = MD_S8Lit("0");
                if (type->underlying_type != 0)
                {
                    MD_String8 underlying_type_name = type->underlying_type->node->string;
                    underlying_type_ptr_expression = MD_S8Fmt(scratch.arena, "&%.*s_type_info",
                                                              MD_S8VArg(underlying_type_name));
                }
                
                int child_count = type->enumerant_count;
                fprintf(out, "TypeInfo %.*s_type_info = "
                        "{TypeKind_Enum, \"%.*s\", %d, %d, %.*s_members, %.*s};\n",
                        MD_S8VArg(type_name),
                        MD_S8VArg(type_name), (int)type_name.size, child_count, MD_S8VArg(type_name),
                        MD_S8VArg(underlying_type_ptr_expression));
            }break;
        }
    }
    
    fprintf(out, "\n");
    
    MD_ReleaseScratch(scratch);
}

void
gen_function_definitions_from_maps(FILE *out, GEN_FileData* filedata)
{
    MD_ArenaTemp scratch = MD_GetScratch(0, 0);
    
    MD_PrintGenNoteCComment(out);
    
    for (GEN_MapInfo *map = filedata->first_map;
         map != 0;
         map = map->next)
    {
        GEN_TypedMapInfo *typed_map = map->typed_map;
        if (typed_map != 0)
        {
            MD_String8 in_type  = typed_map->in_type_string;
            MD_String8 out_type = typed_map->out_type_string;
            
            fprintf(out, "%.*s\n", MD_S8VArg(out_type));
            fprintf(out, "%.*s(%.*s v)\n", MD_S8VArg(map->node->string), MD_S8VArg(in_type));
            fprintf(out, "{\n");
            fprintf(out, "%.*s result;\n", MD_S8VArg(out_type));
            fprintf(out, "switch (v)\n");
            fprintf(out, "{\n");
            
            // default
            fprintf(out, "default:\n");
            fprintf(out, "{\n");
            if (!MD_NodeIsNil(map->default_val))
            {
                MD_String8 default_expr = map->default_val->string;
                MD_String8 val_expr = default_expr;
                if (typed_map->out_is_type_info_ptr)
                {
                    val_expr = MD_S8Fmt(scratch.arena, "&%.*s_type_info", MD_S8VArg(default_expr));
                }
                fprintf(out, "result = %.*s;\n", MD_S8VArg(val_expr));
            }
            else if (typed_map->out_is_type_info_ptr)
            {
                fprintf(out, "result = 0;\n");
            }
            fprintf(out, "}break;\n");
            
            // auto cases
            if (!MD_NodeIsNil(map->auto_val))
            {
                int map_has_an_implicit_case = 0;
                for (GEN_TypeEnumerant *enumerant = typed_map->in->first_enumerant;
                     enumerant != 0;
                     enumerant = enumerant->next)
                {
                    GEN_MapCase *explicit_case = gen_map_case_from_enumerant(map, enumerant);
                    
                    if (explicit_case == 0)
                    {
                        map_has_an_implicit_case = 1;
                        MD_String8 in_expr = enumerant->node->string;
                        fprintf(out, "case %.*s:\n", MD_S8VArg(in_expr));
                    }
                }
                
                if (map_has_an_implicit_case)
                {
                    MD_String8 auto_expr = map->auto_val->string;
                    MD_String8 val_expr = auto_expr;
                    if (typed_map->out_is_type_info_ptr)
                    {
                        val_expr = MD_S8Fmt(scratch.arena, "&%.*s_type_info", MD_S8VArg(auto_expr));
                    }
                    
                    fprintf(out, "{\n");
                    fprintf(out, "result = %.*s;\n", MD_S8VArg(val_expr));
                    fprintf(out, "}break;\n");
                }
            }
            
            // explicit cases
            for (GEN_MapCase *map_case = map->first_case;
                 map_case != 0;
                 map_case = map_case->next)
            {
                MD_String8 in_expr = map_case->in_enumerant->node->string;
                MD_String8 out_expr = map_case->out->string;
                MD_String8 val_expr = out_expr;
                if (typed_map->out_is_type_info_ptr)
                {
                    val_expr = MD_S8Fmt(scratch.arena, "&%.*s_type_info", MD_S8VArg(out_expr));
                }
                
                fprintf(out, "case %.*s:\n", MD_S8VArg(in_expr));
                fprintf(out, "{\n");
                fprintf(out, "result = %.*s;\n", MD_S8VArg(val_expr));
                fprintf(out, "}break;\n");
            }
            
            fprintf(out, "}\n");
            fprintf(out, "return(result);\n");
            fprintf(out, "}\n");
        }
    }
    
    fprintf(out, "\n");
    
    MD_ReleaseScratch(scratch);
}

void print_parse_analysis(GEN_FileData* filedata)
{
    // @notes The generated code doesn't go straight to stdout, and has a lot
    //  of transforms applied. When writing the analyzers, it's often useful to
    //  have a way to directly dump the results of analysis right to stdout to
    //  see what's going on.
    
    // print diagnostics of the parse analysis
    printf("---- PARSE ANALYSIS %.*s ----\n", MD_S8VArg(filedata->filename));
    for (GEN_TypeInfo *type = filedata->first_type;
         type != 0;
         type = type->next)
    {
        char *kind_string = "ERROR";
        switch (type->kind)
        {
            default:break;
            case GEN_TypeKind_Basic:  kind_string = "basic"; break;
            case GEN_TypeKind_Struct: kind_string = "struct"; break;
            case GEN_TypeKind_Enum:   kind_string = "enum"; break;
        }
        
        MD_Node *node = type->node;
        printf("%.*s: %s\n", MD_S8VArg(node->string), kind_string);
        
        // print member lists
        for (GEN_TypeMember *member = type->first_member;
             member != 0;
             member = member->next)
        {
            printf("  %.*s: %.*s\n",
                   MD_S8VArg(member->node->string),
                   MD_S8VArg(member->type->node->string));
        }
        
        // print enumerant lists
        for (GEN_TypeEnumerant *enumerant = type->first_enumerant;
             enumerant != 0;
             enumerant = enumerant->next)
        {
            printf("  %.*s: %d\n",
                   MD_S8VArg(enumerant->node->string),
                   enumerant->value);
        }
    }
    
    for (GEN_MapInfo *map = filedata->first_map;
         map != 0;
         map = map->next)
    {
        MD_Node *node = map->node;
        printf("%.*s: map\n", MD_S8VArg(node->string));
        
        // print case list
        for (GEN_MapCase *map_case = map->first_case;
             map_case != 0;
             map_case = map_case->next)
        {
            printf("  %.*s -> %.*s\n",
                   MD_S8VArg(map_case->in_enumerant->node->string),
                   MD_S8VArg(map_case->out->string));
        }
    }
}


MD_Node* parse_tree_from_file(MD_String8 filename, MD_Arena* arena = nullptr)
{
    MD_ParseResult parse_result = MD_ParseWholeFile(arena, filename);
    // print metadesk errors
    for (MD_Message *message = parse_result.errors.first;
            message != 0;
            message = message->next)
    {
        MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
        MD_PrintMessage(error_file, code_loc, message->kind, message->string);
    }
    return parse_result.node;
}

/*
// MD_ListConcatInPlace(MD_Node *list, MD_Node *to_push)
if (!MD_NodeIsNil(to_push->first_child))
{
    if (!MD_NodeIsNil(list->first_child))
    {
        list->last_child->next = to_push->first_child;
        list->last_child = to_push->last_child;
    }
    else
    {
        list->first_child = to_push->first_child;
        list->last_child = to_push->last_child;
    }
    to_push->first_child = to_push->last_child = MD_NilNode();
}
*/
template <typename T>
void MD_ListConcatInPlace(
    T*& typeinfo_first_1, 
    T*& typeinfo_last_1, 
    T*& typeinfo_first_2, 
    T*& typeinfo_last_2)
{
    if (typeinfo_first_2)
    {
        if (typeinfo_first_1)
        {
            typeinfo_last_1->next = typeinfo_first_2;
            typeinfo_last_1 = typeinfo_last_2;
        }
        else
        {
            typeinfo_first_1 = typeinfo_first_2;
            typeinfo_last_1 = typeinfo_last_2;
        }
    }
}

void MD_MapConcatInPlace(MD_Arena* arena, MD_Map& map1, MD_Map& map2)
{
    for (int i = 0; i < map2.bucket_count; i++)
    {
        // each bucket
        MD_MapBucket& bucket = map2.buckets[i];
        for (MD_MapSlot* slot = bucket.first; slot; slot = slot->next)
        {
            MD_MapInsert(arena, &map1, slot->key, slot->val);
        }
    }
}

void MergeFileData(GEN_FileData& filedata, GEN_FileData& filedata_other)
{
    MD_ListConcatInPlace(filedata.first_type, filedata.last_type, filedata_other.first_type, filedata_other.last_type);
    MD_ListConcatInPlace(filedata.first_map, filedata.last_map, filedata_other.first_map, filedata_other.last_map);
    MD_MapConcatInPlace(filedata.arena, filedata.type_map, filedata_other.type_map);
    MD_MapConcatInPlace(filedata.arena, filedata.map_map, filedata_other.map_map);
}

void CreateDirIfNotExists(MD_String8 dir)
{
    std::string dir_str = std::string((const char*)dir.str, dir.size);
    std::filesystem::path dir_path = std::filesystem::path(dir_str);
    std::filesystem::create_directory(dir_path);
}

GEN_FileData ProcessTypeFile(MD_String8 filename, MD_String8 output_dir, bool is_include = false)
{
    printf("Parsing %.*s\n", MD_S8VArg(filename));
    GEN_FileData filedata = MD_ZERO_STRUCT;
    filedata.arena = MD_ArenaAlloc();
    MD_Arena* arena = filedata.arena;
    filedata.type_map = MD_MapMake(arena);
    filedata.map_map = MD_MapMake(arena);
    filedata.filename = filename;

    // parse phase
    MD_Node* root = parse_tree_from_file(filename, arena);

    // analysis phase
    gen_parse_includes(&filedata, root);
    // Handling includes per-file here. This means there may be redundant work done
    // when multiple files include the same file. 
    // TODO(grayson): this won't work still since it's gonna output the include info too so
    // we'll have duplicate definitions in the output files
    for (MD_String8Node* include = filedata.include_list.first; include; include = include->next)
    {
        GEN_FileData include_filedata = ProcessTypeFile(include->string, output_dir, true);
        // merge our filedata with include filedata somehow
        //MergeFileData(filedata, include_filedata);
        if (!filedata.include_filedata)
        {
            GEN_FileData* new_include_filedata = (GEN_FileData*)MD_ArenaPush(arena, sizeof(GEN_FileData));
            *new_include_filedata = include_filedata;
            filedata.include_filedata = new_include_filedata;
        }
        else
        {
            MergeFileData(*filedata.include_filedata, include_filedata);
        }
    }

    gen_gather_types_and_maps(&filedata, root);
    gen_check_duplicate_member_names(&filedata);
    gen_equip_basic_type_size(&filedata);
    gen_equip_struct_members(&filedata);
    gen_equip_enum_underlying_type(&filedata);
    gen_equip_enum_members(&filedata);
    gen_equip_map_in_out_types(&filedata);
    gen_equip_map_cases(&filedata);
    gen_check_duplicate_cases(&filedata);
    gen_check_complete_map_cases(&filedata);

    if (!is_include)    
    {
        MD_String8 filename_noext = remove_file_extension(filename);
        MD_String8 header_name = MD_S8Fmt(arena, "%.*s.h", MD_S8VArg(filename_noext));
        MD_String8 header_path = MD_S8Fmt(arena, "%.*s/%.*s", MD_S8VArg(output_dir), MD_S8VArg(header_name));
        MD_String8 source_filename = MD_S8Fmt(arena, "%.*s/%.*s.cpp", MD_S8VArg(output_dir), MD_S8VArg(filename_noext));

        // generate header file
        {
            FILE *h = fopen((const char*)header_path.str, "wb");
            MD_String8 header_guard_name = MD_S8Stylize(arena, filename_noext, MD_IdentifierStyle_UpperCase, MD_S8Lit(""));
            fprintf(h, "#if !defined(%.*s_H)\n", MD_S8VArg(header_guard_name));
            fprintf(h, "#define %.*s_H\n", MD_S8VArg(header_guard_name));
            // need type info for all type headers/source
            fprintf(h, "#include \"type_info.h\"\n");
            gen_output_include_statements(h, &filedata);
            gen_type_definitions_from_types(h, &filedata);
            // forward declare typeinfo so we can only include it in the cpp
            gen_function_declarations_from_maps(h, &filedata);
            gen_type_info_declarations_from_types(h, &filedata);
            fprintf(h, "#endif // %.*s_H\n", MD_S8VArg(header_guard_name));
            fclose(h);
        }
        
        // generate definitions file
        {
            FILE *c = fopen((const char*)source_filename.str, "wb");
            fprintf(c, "#include \"%.*s\"\n", MD_S8VArg(header_name));
            gen_struct_member_tables_from_types(c, &filedata);
            gen_enum_member_tables_from_types(c, &filedata);
            gen_type_info_definitions_from_types(c, &filedata);
            gen_function_definitions_from_maps(c, &filedata);
            fclose(c);
        }

        print_parse_analysis(&filedata);
    }

    return filedata;
}



//~ main //////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
    // setup the global arena
    //arena = MD_ArenaAlloc();
    
    // output stream routing
    error_file = stderr;
    
    //printf("Recursive dirs\n");
    //for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("C:/Dev/tiny_engine/src/types/metadesk"))
    //    std::cout << dirEntry << std::endl;
    //printf("Recursive dirs END\n");

    MD_String8 output_dir = MD_S8Lit("../generated");
    // TESTING
    std::filesystem::remove_all(std::filesystem::path("../generated"));
    // ----
    CreateDirIfNotExists(output_dir);
    // parse all files passed to the command line
    //MD_Node *list = MD_MakeList(arena);
    for (int i = 1; i < argc; i += 1)
    {
        // parse the file
        MD_String8 file_name = MD_S8CString(argv[i]);
        ProcessTypeFile(file_name, output_dir);
    }

    return 0;
}