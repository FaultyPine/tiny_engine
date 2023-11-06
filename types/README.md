
---------------------------------
# Metadesk reflection system notes:

type_metadata.cpp and type_metadata.h contain the actual code for generating 

compiling type.metadata.cpp (see build.bat) and running the output exe will generate type files in a generated/ folder


-----------------------------

## Usage

When specifying types in a .type file, the syntax is as follows

To declare a new type, use `@type`

Declaring primitive types:
`@type(basic, unsigned char) u8: 1;`
the `basic` denotes that this type compiles out to a typedef
the 2nd argument denotes the actual type being defined.
The token after the `(..., ...)` (in this case, `u8`) defines the given
name for this type.
The number after the type's name denotes the size in bytes of this type


### Declaring custom structures:

Declaring a custom structure looks something like this:
```C
@type(struct)
Vector2:
{
  x: f32;
  y: f32;
}
```
using `@type(struct)` begins a struct definition
You then define the name of the structure followed by `:` and curly braces `{}`
Inside the braces you can denote each member of the struct like: 
`name: type`

You can also define in-place arrays as structure members like:
```C
@type(struct)
Mat2x2:
{
    data: @array(2,2) f32;
}
```
In this case, this structure will output
```C
struct Mat2x2
{
    f32 data[2][2];
};
```

### Including other type files


To access types declared in other files, use the `@include` tag
```C
// contains definition for `f32` that looks like `@type(basic, float) f32: 4;`
@include "basics.type" 

@type(struct)
SomeStruct:
{
    member: f32;
}
```
Of note: the include path is always relative to the root folder where all the .type files are. So if your input types folder is `types`, and you have a file at `types/game/enemy/enemy.type`, and you are writing a type definition in `types/game/world.type`, you would need to include it like `@include "game/enemy/enemy.type"`

### Declaring type maps

You can declare a mapping from an enum to another type like so:
```C
@type(enum: U32)
Shape:
{
  Circle: 1,
  Segment,
  Polygon,
}

@type(struct)
Circle:
{
  r: F32;
  pos: V2F32;
}

// .. imagine RoundedSegment and RoundedPolygon definitions here

@map(Shape -> `$Type`; complete)
type_info_from_shape:
{
  Circle -> Circle,
  Segment -> RoundedSegment,
  Polygon -> RoundedPolygon,
}

@map(Shape -> U32; default: 0; auto: 64)
max_slot_from_shape:
{
  Polygon -> 12,
}
```
Note that the first example declares a mapping from the Shape enum
to *any arbitrary type*. Also note the `complete` tag in the `@map` declaration - this denotes that this mapping should always cover all cases of this enum. In the other case, in `max_slot_from_shape` we map only the polygon type to 12. Other types will get a mapping of 64, and any types that we try to convert that are not a valid Shape will get the default 0.

To get a better idea of what these declarations mean, here is compiled type info from `type_info_from_shape`:
```C
TypeInfo* type_info_from_shape(Shape v)
{
    TypeInfo* result;
    switch (v)
    {
        default:
        {
            result = 0;
        } break;
        case Shape_Circle:
        {
            result = &Circle_type_info;
        } break;
        case Shape_Segment:
        {
            result = &RoundedSegment_type_info;
        } break;
        case Shape_Polygon:
        {
            result = &RoundedPolygon_type_info;
        } break;
    }
    return result;
}
```

and here it is for `max_slot_from_shape`:
```C
U32 max_slot_from_shape(Shape v)
{
    U32 result;
    switch (v)
    {
        default:
        {
            result = 0;
        } break;
        case Shape_Circle:
        case Shape_Segment:
        {
            result = 64;
        } break;
        case Shape_Polygon:
        {
            result = 12;
        } break;
    }
    return result;
}
```
