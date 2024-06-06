/*
 * Converted from the original LUA script to a module for Azerothcore (Sunwell)
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Configuration/Config.h"
#include "Chat.h"

void attemptToApplyRandomEnchantment(std::string event, Player *player, Item *item);
[[nodiscard]] std::string getItemClassString(uint32 itemClass);
[[nodiscard]] int32 getRandomItemRarity(uint32 itemQuality);
[[nodiscard]] int8 getItemTier(int32 itemRarity);
[[nodiscard]] int32 getEnchantment(int32 tier, std::string classString, uint32 exclusiveSubClass);

class RandomEnchantsPlayer : public PlayerScript
{
public:
    RandomEnchantsPlayer() : PlayerScript("RandomEnchantsPlayer") {}

    void OnLogin(Player *player) override;
    void OnLootItem(Player *player, Item *item, uint32 /*count*/, ObjectGuid /*lootguid*/) override;
    void OnCreateItem(Player *player, Item *item, uint32 /*count*/) override;
    void OnQuestRewardItem(Player *player, Item *item, uint32 /*count*/) override;
    void OnGroupRollRewardItem(Player *player, Item *item, uint32 /*count*/, RollVote /*voteType*/, Roll * /*roll*/) override;
};

void AddRandomEnchantsScripts()
{
    new RandomEnchantsPlayer();
}
