#include "pch.h"
#include "guy.h"
#include "tiny_engine/tiny_fs.h"



void MakeGuyGroup(GuyGroup* group) {
    group->sprite = Sprite(LoadTexture(ResPath("other/awesomeface.png")));
}
void DrawGuyGroup(GuyGroup* group) {
    for (Guy& guy : group->guys) {
        DrawGuy(group, &guy);
    }
}

void MakeGuy(GuyGroup* group, glm::vec2 pos) {
    Guy guy;
    guy.tf = Transform2D(pos, glm::vec2(50));
    group->guys.push_back(guy);
}

void DrawGuy(GuyGroup* group, Guy* guy) {
    group->sprite.DrawSprite(guy->tf);
}



