#include "components/simple_scene.h"
#include <memory>

namespace m1
{
    enum CanonType
    {
        CANON_ORANGE=0,
        CANON_BLUE=1,
        CANON_YELLOW=2,
        CANON_PURPLE=3,
        EMPTY=-1
    };

    struct Canon {
        CanonType type;
        int xPos;
        int yPos;
        std::string meshId;
        bool isDestroying;
        float destroyScale;
        float reloadTime;
        Canon(int _xPos, int _yPos, std::string _meshId, CanonType _type);
    };
    
    struct Star {
        std::string meshId;
        int xPos;
        int yPos;
        bool enabled;
    };

    struct Bullet {
        int lane;
        float xPos;
        float angle;
        std::string meshId;
        CanonType type;
    };

    struct Enemy {
        int health=0;
        int lane;
        float xPos;
        float destroyScale;
        CanonType vulnerability;
        std::string meshId;
    };

    struct EnemySpawner {
        float timer;
        int activeEnemies;
        std::vector<Enemy> enemies;
        inline Enemy* SpawnEnemy(int xRes);
        inline void AdvanceEnemies(float deltaTimeSeconds);
        inline bool isEnemy(int lane, CanonType type);
    };

    struct StarSpawner {
        std::vector<Star> stars;
        float cooldown;
        int activeStars;
        void RefreshStars(int, int);
        Star* getStar(int xPos, int yPos);
    };

    struct PlayerCursor {
        int xPos;
        int yPos;
        CanonType holding;
        PlayerCursor(CanonType _type);
        PlayerCursor();
    };

    struct Player {
        int life;
        int stars;
        std::string meshId;
        Player();
        Player(int _life, int _stars, std::string _meshId);
    };

    struct ShopItem {
        CanonType item;
        int cost;
        std::string meshId;
        ShopItem(CanonType _item, std::string _meshId);
    };

    struct Shop {
        std::vector<ShopItem> items;
        std::string meshId;
        Shop(std::string _meshId);
        Shop();
    };

    struct BuildingZone {
        std::shared_ptr<Canon> canon;
        glm::vec3 position;
        std::string meshId;
        BuildingZone(glm::vec3 _position, std::string _meshId);
    };

    struct DamageZone {
        glm::vec3 zonePos;
        std::string meshId;
        DamageZone(glm::vec3 _zonePos, std::string _meshId);
        DamageZone();
    };
    

    class Tema1 : public gfxc::SimpleScene
    {
     public:
        Tema1();
        ~Tema1();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        inline void CreateCanon(std::shared_ptr<Canon> canon);
        inline void CreateCanon(int xPos, int yPos, CanonType type, std::string name);
        inline void CreatePlayerInfo();
        inline void CreateEnemy(Enemy *bob);
        inline void CreateBullet(Bullet *pew);

        inline void RenderGrid();
        inline void RenderDamageZone();
        inline void RenderCanon(Canon *canon);
        inline void RenderCanon(std::string id);
        inline void RenderShop();
        inline void RenderPlayerInfo();
        inline void RenderPlayerCursor();
        inline void RenderStars();
        inline void RenderEnemies();
        inline void RenderCanon();
        inline void RenderBullets();

        inline int GetItemIndex();
        inline std::tuple<int, int> GetSquareIndex(int xPos, int yPos);
        inline void CheckForHit();
        inline void FireBullets(float deltaTimeSeconds);
        inline void AdvanceBullets(float deltaTimeSeconds);

     protected:
        glm::mat3 modelMatrix;
        std::vector<std::vector<BuildingZone>> buildingZones;
        DamageZone damageZone;
        Shop shop;
        Player player;
        PlayerCursor cursor;
        StarSpawner pointSpawner;
        EnemySpawner enemySpawner;
        std::vector<Bullet> bullets;
    };
}   // namespace m1
