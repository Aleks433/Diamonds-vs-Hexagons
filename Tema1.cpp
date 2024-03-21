#include "tema1.h"


#include <vector>
#include <iostream>

#include "lab_m1/lab3/object2D.h"
#include "transform2D.h"

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema1::Tema1()
{}

Tema1::~Tema1()
{
}

Canon::Canon(int _xPos, int _yPos, std::string _meshId, CanonType _type) :
    xPos(_xPos),
    yPos(_yPos),
    meshId(_meshId),
    type(_type),
    destroyScale(1),
    isDestroying(false),
    reloadTime(0)
{}

Enemy* EnemySpawner::SpawnEnemy(int xRes) {
    activeEnemies++;
    timer = 0;
    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].health == 0) {
            enemies[i].health = 3;
            enemies[i].lane = rand() % 3;
            enemies[i].xPos = xRes+75;
            enemies[i].destroyScale = 1;
            enemies[i].vulnerability = static_cast<CanonType>(rand() % 4);
            return &enemies[i];
        }
    }
}
inline bool EnemySpawner::isEnemy(int lane, CanonType type) {

    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].health != 0 && enemies[i].lane == lane &&
            type == enemies[i].vulnerability)
            return true;
    }
    return false;
}
inline void EnemySpawner::AdvanceEnemies(float deltaTimeSeconds) {
    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].health != 0) {
            enemies[i].xPos -= 50*deltaTimeSeconds;
        }
    }
}

void StarSpawner::RefreshStars(int resX, int resY) {
    cooldown = 0;
    activeStars = 3;
    for (int i = 0; i < activeStars; i++) {
        stars[i].enabled = true;
        stars[i].xPos = abs(rand() % resX - 50);
        stars[i].yPos = abs(rand() % resY - 50);
    }
}

Star* StarSpawner::getStar(int xPos, int yPos) {
    for (int i = 0; i < stars.size(); i++) {
        if (xPos >= stars[i].xPos && xPos <= stars[i].xPos + 50 &&
            yPos >= stars[i].yPos && yPos <= stars[i].yPos + 50) {
            return &stars[i];
        }
    }
    return NULL;
}

PlayerCursor::PlayerCursor(CanonType _type) : 
    holding(_type)
{}

PlayerCursor::PlayerCursor() :
    holding(EMPTY)
{}

ShopItem::ShopItem(CanonType _item, std::string _meshId):
    item(_item),
    meshId(_meshId)
{
    switch (item)
    {
    case CANON_ORANGE:
        cost = 1;
        break;
    case CANON_BLUE:
        cost = 2;
        break;
    case CANON_YELLOW:
        cost = 2;
        break;
    case CANON_PURPLE:
        cost = 3;
        break;
    case EMPTY:
        break;
    default:
        break;
    }
}

Shop::Shop(std::string _meshId) :
    meshId(_meshId)
{}

Shop::Shop(){}

BuildingZone::BuildingZone(glm::vec3 _position, std::string _meshId) :
    position(_position),
    meshId(_meshId),
    canon(nullptr)
{}

Player::Player(int _life, int _stars, std::string _meshId) :
    life(_life),
    stars(_stars),
    meshId(_meshId)
{}
Player::Player() {}

DamageZone::DamageZone(glm::vec3 _zonePos, std::string _meshId) :
    zonePos(_zonePos),
    meshId(_meshId)
{}
DamageZone::DamageZone(){}



void Tema1::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    float squareSide = 125;

    //Square grid init
    for (int i = 0; i < 3; i++) {
        std::vector<BuildingZone> aux;
        for (int j = 0; j < 3; j++) {
            int squareIndex = i * 10 + j;
            aux.push_back(BuildingZone(glm::vec3(155 * i + 75, 155 * j + 25, 0), "buildingZone" + std::to_string(squareIndex)));
            Mesh* square = object2D::CreateSquare(aux[j].meshId, aux[j].position, squareSide, glm::vec3(0.2, 1, 0), true);
            AddMeshToList(square);
        }
        buildingZones.push_back(aux);
    }

    //Damage zone init
    float rectangleLength = 40;
    float rectangleWidth = 435;
    damageZone = DamageZone(glm::vec3(10, 25, 0), "damageZone");
    Mesh* rectangle = object2D::CreateRectangle(damageZone.meshId, damageZone.zonePos, rectangleLength, rectangleWidth, glm::vec3(1, 0, 0), true);
    AddMeshToList(rectangle);

    //shop init
    shop = Shop("canonShop");
    for (int i = 0; i < 4; i++) {
        CanonType type = static_cast<CanonType>(i);
        glm::vec3 bottomLeft = glm::vec3(i * 155 + 55, 575, 0);
        Mesh* border = object2D::CreateSquare(shop.meshId + std::to_string(i), bottomLeft, 125, glm::vec3(1, 1, 1));
        AddMeshToList(border);

        ShopItem item(type, "item" + std::to_string(i));
        CreateCanon(bottomLeft[0], bottomLeft[1], type, item.meshId);
        for(int j = 0; j<item.cost;j++) {
            Mesh* star = object2D::CreateStar(item.meshId + "star" + std::to_string(j), bottomLeft + glm::vec3(j * 50, -50, 0), 25, glm::vec3(1, 1, 0), true);
            AddMeshToList(star);
        }
        shop.items.push_back(item);
    }

    CreatePlayerInfo();

    for (int i = 0; i < 3; i++) {
        Star aux;
        aux.meshId = "star" + std::to_string(i);
        Mesh* star = object2D::CreateStar(aux.meshId, glm::vec3(0, 0, 0), 50, glm::vec3(0.01,0.95 , 0.98), true);
        AddMeshToList(star);
        pointSpawner.stars.push_back(aux);
    }
    pointSpawner.RefreshStars(resolution.x, resolution.y);

    for(int i=0;i<3;i++) {
        Enemy aux;
        aux.xPos = resolution.x;
        aux.meshId = "enemy" + std::to_string(i);
        enemySpawner.enemies.push_back(aux);
    }
    enemySpawner.timer = 2;
    enemySpawner.activeEnemies = 0;
    //test
    Mesh* square = object2D::CreateSquare("square", glm::vec3(0, 0, 0), 50, glm::vec3(1, 1, 1), true);
    AddMeshToList(square);
}


void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema1::Update(float deltaTimeSeconds)
{
    pointSpawner.cooldown += deltaTimeSeconds;
    if (floor(pointSpawner.cooldown) > 7) {
        glm::ivec2 resolution = window->GetResolution();
        pointSpawner.RefreshStars(resolution.x, resolution.y);
    }
    if (enemySpawner.activeEnemies < 3) {
        enemySpawner.timer += deltaTimeSeconds;
        if (floor(enemySpawner.timer) > 5) {
            std::cout << "spawning enemy...";
            glm::ivec2 resolution = window->GetResolution();
            Enemy *bob = enemySpawner.SpawnEnemy(resolution.x);
            CreateEnemy(bob);
        }
    }

    enemySpawner.AdvanceEnemies(deltaTimeSeconds);
    AdvanceBullets(deltaTimeSeconds);
    CheckForHit();
    FireBullets(deltaTimeSeconds);

    RenderBullets();
    RenderEnemies();
    RenderStars();
    RenderPlayerCursor();
    RenderGrid();
    RenderDamageZone();
    RenderShop();
    RenderPlayerInfo();
}


void Tema1::FrameEnd()
{
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
}


void Tema1::OnKeyPress(int key, int mods)
{
    // Add key press event
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
    cursor.xPos = mouseX;
    cursor.yPos = window->GetResolution().y - mouseY;
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_2) {
        int item;
        if ((item = GetItemIndex()) != -1) {
            int cost=0;
            CanonType canonType = static_cast<CanonType>(item);
            switch (canonType)
            {
            case CANON_ORANGE:
                cost = 1;
                break;
            case CANON_BLUE:
                cost = 2;
                break;
            case CANON_YELLOW:
                cost = 2;
                break;
            case CANON_PURPLE:
                cost = 3;
                break;
            case EMPTY:
                break;
            default:
                break;
            }
            if (player.stars >= cost) {
                cursor.holding = canonType;
                CreateCanon(0, 0, cursor.holding, "mouse");
            }
        }
        Star* star;
        if ((star = pointSpawner.getStar(cursor.xPos, cursor.yPos)) != NULL) {
            if (star->enabled) {
                star->enabled = false;
                pointSpawner.activeStars--;
                player.stars++;
            }
        }
    }
    if (button == GLFW_MOUSE_BUTTON_3) {
        int xSqare, ySquare;
        tie(xSqare, ySquare) = GetSquareIndex(cursor.xPos, cursor.yPos);
        if (xSqare != -1) {
            buildingZones[xSqare][ySquare].canon->isDestroying = true;
            buildingZones[xSqare][ySquare].canon->destroyScale = 1;
        }
        
    }
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
    if (button == GLFW_MOUSE_BUTTON_2 && cursor.holding != EMPTY) {
        int xSqare, ySquare;
        tie(xSqare, ySquare) = GetSquareIndex(cursor.xPos, cursor.yPos);
        if (xSqare != -1) {
            int cost = 0;
            switch (cursor.holding)
            {
            case CANON_ORANGE:
                cost = 1;
                break;
            case CANON_BLUE:
                cost = 2;
                break;
            case CANON_YELLOW:
                cost = 2;
                break;
            case CANON_PURPLE:
                cost = 3;
                break;
            case EMPTY:
                break;
            default:
                break;
            }
            player.stars -= cost;
            buildingZones[xSqare][ySquare].canon = make_shared<Canon>(xSqare, ySquare, "canon" + std::to_string(xSqare) + std::to_string(ySquare), cursor.holding);
            CreateCanon(buildingZones[xSqare][ySquare].canon);
        }
        cursor.holding = EMPTY;
    }
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema1::OnWindowResize(int width, int height)
{
}


inline void Tema1::RenderGrid() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (buildingZones[i][j].canon != nullptr) {
                if (ceil(buildingZones[i][j].canon->destroyScale) == 0) {
                    buildingZones[i][j].canon = nullptr;
                }
                else {
                    RenderCanon(buildingZones[i][j].canon.get());
                }
            }
            modelMatrix = glm::mat3(1);
            RenderMesh2D(meshes[buildingZones[i][j].meshId], shaders["VertexColor"], modelMatrix);
        }
    }
    
}

inline void Tema1::RenderDamageZone() {
    modelMatrix = glm::mat3(1);
    RenderMesh2D(meshes[damageZone.meshId], shaders["VertexColor"], modelMatrix);
}

inline void Tema1::RenderCanon(Canon *canon) {
    modelMatrix = glm::mat3(1);
    if (canon->isDestroying) {
        int xPos, yPos;
        xPos = canon->xPos * 155 + 75;
        yPos = canon->yPos * 155 + 25;
        canon->destroyScale -= 0.01;
        modelMatrix *= transform2D::Translate(xPos + 50, yPos + 50);
        modelMatrix *= transform2D::Scale(canon->destroyScale, canon->destroyScale);
        modelMatrix *= transform2D::Translate(-xPos - 50, -yPos - 50);
    }
    RenderMesh2D(meshes[canon->meshId + "body"], shaders["VertexColor"], modelMatrix);
    RenderMesh2D(meshes[canon->meshId + "barrel"], shaders["VertexColor"], modelMatrix);
}

inline void Tema1::RenderCanon(std::string id) {
    modelMatrix = glm::mat3(1);
    RenderMesh2D(meshes[id + "body"], shaders["VertexColor"], modelMatrix);
    RenderMesh2D(meshes[id + "barrel"], shaders["VertexColor"], modelMatrix);
}

inline void Tema1::RenderShop() {
    modelMatrix = glm::mat3(1);
    for (int i = 0; i < shop.items.size(); i++) {
        RenderMesh2D(meshes[shop.meshId + std::to_string(i)], shaders["VertexColor"], modelMatrix);
        RenderCanon(shop.items[i].meshId);
        for (int j = 0; j < shop.items[i].cost; j++) {
            RenderMesh2D(meshes[shop.items[i].meshId + "star" + std::to_string(j)], shaders["VertexColor"], modelMatrix);
        }
    }
}

inline void Tema1::RenderPlayerCursor() {
    if (cursor.holding != EMPTY) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(cursor.xPos, cursor.yPos);
        RenderMesh2D(meshes["mousebody"], shaders["VertexColor"], modelMatrix);
        RenderMesh2D(meshes["mousebarrel"], shaders["VertexColor"], modelMatrix);
    }
}

inline void Tema1::CreateCanon(std::shared_ptr<Canon> canon) {
    glm::vec3 side = glm::vec3(155 * canon->xPos + 75, 155 * canon->yPos + 25, 0);
    glm::vec3 color=glm::vec3(0,0,0);    
    switch (canon->type) {
        case CANON_ORANGE: {
            color = glm::vec3(1, 0.3, 0);
            break;
        }
        case CANON_BLUE: {
            color = glm::vec3(0, 0, 1);
            break;
        }
        case CANON_YELLOW: {
            color = glm::vec3(1, 1, 0);
            break;
        }
        case CANON_PURPLE: {
            color = glm::vec3(1, 0, 1);
            break;
        }
    }
    int length = 125;
    Mesh* diamond = object2D::CreateDiamond(canon->meshId + "body", side, length, color, true);
    Mesh* rectangle = object2D::CreateRectangle(canon->meshId + "barrel", side + glm::vec3( 0.65 * length, 125 / 3, 0), length/3, length / 3, color, true);
    AddMeshToList(diamond);
    AddMeshToList(rectangle);
}

inline void Tema1::CreateCanon(int xPos, int yPos, CanonType type, std::string name) {
    glm::vec3 side = glm::vec3(xPos, yPos, 0);
    glm::vec3 color=glm::vec3(0,0,0);    
    switch (type) {
        case CANON_ORANGE: {
            color = glm::vec3(1, 0.3, 0);
            break;
        }
        case CANON_BLUE: {
            color = glm::vec3(0, 0, 1);
            break;
        }
        case CANON_YELLOW: {
            color = glm::vec3(1, 1, 0);
            break;
        }
        case CANON_PURPLE: {
            color = glm::vec3(1, 0, 1);
            break;
        }
    }
    int length = 125;
    Mesh* diamond = object2D::CreateDiamond(name + "body", side, length, color, true);
    Mesh* rectangle = object2D::CreateRectangle(name + "barrel", side + glm::vec3(0.65 * length, 125 / 3, 0), length / 3, length / 3, color, true);
    AddMeshToList(diamond);
    AddMeshToList(rectangle);
}

inline void Tema1::RenderPlayerInfo() {
    modelMatrix = glm::mat3(1);
    for (int i = 0; i < player.life; i++) {
        RenderMesh2D(meshes[player.meshId + "life" + std::to_string(i)], shaders["VertexColor"], modelMatrix);
    }
    for (int i = 0; i < player.stars; i++) {
        RenderMesh2D(meshes[player.meshId + "star" + std::to_string(i)], shaders["VertexColor"], modelMatrix);
    }

}

inline void Tema1::RenderStars() {
    for (int i = 0; i < pointSpawner.stars.size(); i++) {
        if (pointSpawner.stars[i].enabled) {
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(pointSpawner.stars[i].xPos, pointSpawner.stars[i].yPos);
            RenderMesh2D(meshes[pointSpawner.stars[i].meshId], shaders["VertexColor"], modelMatrix);
        }
    }
}

inline void Tema1::RenderEnemies() {
    for (int i = 0; i < enemySpawner.enemies.size(); i++) {
        if (enemySpawner.enemies[i].health != 0) {
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(enemySpawner.enemies[i].xPos, enemySpawner.enemies[i].lane * 155 + 87.5);
            RenderMesh2D(meshes[enemySpawner.enemies[i].meshId + "inner"], shaders["VertexColor"], modelMatrix);
            RenderMesh2D(meshes[enemySpawner.enemies[i].meshId + "outer"], shaders["VertexColor"], modelMatrix);
        }
        else if (enemySpawner.enemies[i].destroyScale > 0) {
            int xPos = enemySpawner.enemies[i].xPos;
            int yPos = enemySpawner.enemies[i].lane * 155 + 87.5;
            enemySpawner.enemies[i].destroyScale -= 0.01;
            modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(xPos, yPos);
            modelMatrix *= transform2D::Scale(enemySpawner.enemies[i].destroyScale, enemySpawner.enemies[i].destroyScale);
            RenderMesh2D(meshes[enemySpawner.enemies[i].meshId + "inner"], shaders["VertexColor"], modelMatrix);
            RenderMesh2D(meshes[enemySpawner.enemies[i].meshId + "outer"], shaders["VertexColor"], modelMatrix);
        }
    }
}

inline void Tema1::RenderBullets() {
    for (int i = 0; i < bullets.size(); i++) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(bullets[i].xPos+25, bullets[i].lane * 155 + 50 + 25);
        modelMatrix *= transform2D::Rotate(bullets[i].angle);
        modelMatrix *= transform2D::Translate(-25, -25);
        bullets[i].angle += 0.1;
        RenderMesh2D(meshes[bullets[i].meshId], shaders["VertexColor"], modelMatrix);
    }

}

inline void Tema1::CreateBullet(Bullet* pew) {
    glm::vec3 color;
    switch (pew->type) {
        case CANON_ORANGE: {
            color = glm::vec3(1, 0.3, 0);
            break;
        }
        case CANON_BLUE: {
            color = glm::vec3(0, 0, 1);
            break;
        }
        case CANON_YELLOW: {
            color = glm::vec3(1, 1, 0);
            break;
        }
        case CANON_PURPLE: {
            color = glm::vec3(1, 0, 1);
            break;
        }
    }
    Mesh* bullet = object2D::CreateStar(pew->meshId, glm::vec3(0, 0, 0), 50, color, true);
    AddMeshToList(bullet);
}

inline void Tema1::CreateEnemy(Enemy *bob) {
    Mesh* inner = object2D::CreateHexagon(bob->meshId + "inner", glm::vec3(0,0,0), 25, glm::vec3(0.64, 0.74, 0.74), true);

    glm::vec3 color=glm::vec3(0,0,0);    
    switch (bob->vulnerability) {
        case CANON_ORANGE: {
            color = glm::vec3(1, 0.3, 0);
            break;
        }
        case CANON_BLUE: {
            color = glm::vec3(0, 0, 1);
            break;
        }
        case CANON_YELLOW: {
            color = glm::vec3(1, 1, 0);
            break;
        }
        case CANON_PURPLE: {
            color = glm::vec3(1, 0, 1);
            break;
        }
    }
    Mesh* outer = object2D::CreateHexagon(bob->meshId + "outer", glm::vec3(0,0, 0), 50, color, true);
    AddMeshToList(inner);
    AddMeshToList(outer);
}

inline void Tema1::CreatePlayerInfo() {
    player = Player(3, 3, "player");
    glm::vec3 playerHudPos = glm::vec3(750, 575, 0);
    for (int i = 0; i < player.life;i++) {
        Mesh* life = object2D::CreateSquare(player.meshId + "life" + std::to_string(i), playerHudPos + glm::vec3(i * 155, 0, 0), 125, glm::vec3(1, 0, 0), true);
        AddMeshToList(life);
    }
    for (int i = 0; i < 10; i++) {
        Mesh* star = object2D::CreateStar(player.meshId + "star" + std::to_string(i), playerHudPos + glm::vec3(i * 50, -50, 0), 25, glm::vec3(1, 1, 0), true);
        AddMeshToList(star);
    }
}

inline int Tema1::GetItemIndex() {
    for (int i = 0; i < 4; i++) {
        int xBound = i * 155 + 55;
        int yBound = 575;
        if (cursor.xPos >= xBound && cursor.xPos <= xBound + 125 &&
            cursor.yPos >= yBound && cursor.yPos <= yBound + 125) {
            return i;
        }
    }
    return -1;
}

inline std::tuple<int, int> Tema1::GetSquareIndex(int xPos, int yPos) {
    for (int i = 0; i < buildingZones.size(); i++) {
        for (int j = 0; j < buildingZones[i].size(); j++) {
            int xBound = 155 * i + 75;
            int yBound = 155 * j + 25;
            if (xPos >= xBound && xPos <= xBound + 125 &&
                yPos >= yBound && yPos <= yBound + 125) {
                return std::make_tuple(i, j);
            }
        }
    }
    return std::make_tuple(-1, -1);
}

inline void Tema1::CheckForHit() {
    for (int i = 0; i < enemySpawner.enemies.size(); i++) {
        Enemy* enemy = &enemySpawner.enemies[i];
        if (enemy->health != 0) {
            for (int j = 0; j < bullets.size(); j++) {
                if (bullets[j].lane == enemy->lane && bullets[j].type == enemy->vulnerability &&
                    bullets[j].xPos+100 >= enemy->xPos) {
                    enemy->health--;
                    if (enemy->health == 0) {
                        enemySpawner.activeEnemies--;
                    }
                    bullets.erase(bullets.begin() + j);
                }
            }
            int xSquare, ySquare;
            tie(xSquare, ySquare) = GetSquareIndex(enemy->xPos-130, enemy->lane * 155 + 87.5);
            if (xSquare != -1) {
                if (buildingZones[xSquare][ySquare].canon != nullptr) {
                    buildingZones[xSquare][ySquare].canon->isDestroying = true;
                    buildingZones[xSquare][ySquare].canon->destroyScale = 1;
                }
            }
            if (enemy->xPos <= damageZone.zonePos[0]) {
                player.life--;
                enemy->health = 0;
                enemySpawner.activeEnemies--;
                if (player.life == 0) {
                    window->Close();
                }
            }
        }
    }
}

inline void Tema1::FireBullets(float deltaTimeSeconds) {
    for (int i = 0; i < buildingZones.size(); i++) {
        for (int j = 0; j < buildingZones[i].size(); j++) {

            if (buildingZones[i][j].canon != nullptr && !buildingZones[i][j].canon->isDestroying) {

                buildingZones[i][j].canon->reloadTime -= deltaTimeSeconds;
                if (ceil(buildingZones[i][j].canon->reloadTime) < 0) {

                    if (enemySpawner.isEnemy(j, buildingZones[i][j].canon->type)) {

                        buildingZones[i][j].canon->reloadTime = 1.5;
                        Bullet aux;
                        aux.lane = j;
                        aux.xPos = i * 155 + 75 + 125;
                        aux.angle = 0;
                        aux.type = buildingZones[i][j].canon->type;
                        aux.meshId = "bullet" + std::to_string(i) + std::to_string(j); 
                        CreateBullet(&aux);
                        bullets.push_back(aux);
                    }
                }
            }
        }
    }
}

inline void Tema1::AdvanceBullets(float deltaTimeSeconds) {
    for (int i = 0; i < bullets.size(); i++) {
        bullets[i].xPos += deltaTimeSeconds * 150;
    }
}
