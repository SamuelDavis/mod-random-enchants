#include "random_enchants.h"

void attemptToApplyRandomEnchantment(std::string event, Player *player, Item *item)
{
    const uint8 MAX_ENCHANT_CHANCES = 3;
    bool isEnabled = sConfigMgr->GetOption<bool>("RandomEnchants.Enable", true) && sConfigMgr->GetOption<bool>(event, true);
    if (!isEnabled)
        return;

    bool debugIsEnabled = sConfigMgr->GetOption<bool>("RandomEnchants.Debug", false);

    ChatHandler chatHandle = ChatHandler(player->GetSession());
    const ItemTemplate *itemTemplate = item->GetTemplate();
    uint32 itemId = itemTemplate->ItemId;
    uint32 itemQuality = itemTemplate->Quality;
    uint32 itemClass = itemTemplate->Class;
    uint32 itemSubClass = itemTemplate->SubClass;
    uint32 itemLevel = itemTemplate->ItemLevel; // use for rarity?
    std::string itemClassString = getItemClassString(itemClass);
    std::string itemName = itemTemplate->Name1;
    uint8 enchantmentSlotIterator = PROP_ENCHANTMENT_SLOT_0;

    if (debugIsEnabled)
        chatHandle.PSendSysMessage(
            "{\n  name: %s,\n  id: %i,\n  quality: %i,\n  class: %i,\n  subClass: %i,\n  classString: %s,\n  level: %i\n}",
            itemName, itemId, itemQuality, itemClass, itemSubClass, itemClassString == "" ? "missing" : itemClassString, itemLevel);

    if (itemClassString == "")
        return;

    float enchantChanceThresholds[MAX_ENCHANT_CHANCES] = {
        sConfigMgr->GetOption<float>("RandomEnchants.EnchantChance1", 70.0f),
        sConfigMgr->GetOption<float>("RandomEnchants.EnchantChance2", 65.0f),
        sConfigMgr->GetOption<float>("RandomEnchants.EnchantChance3", 60.0f),
    };

    size_t i = 0;
    for (; i < MAX_ENCHANT_CHANCES; i++)
    {
        double chance = rand_chance();
        float threshold = enchantChanceThresholds[i];
        int32 itemRarity = getRandomItemRarity(itemQuality);
        int8 itemTier = getItemTier(itemRarity);
        int32 enchantment = getEnchantment(itemTier, itemClassString, itemSubClass);
        bool enchantmentExists = sSpellItemEnchantmentStore.LookupEntry(enchantment);
        for (; enchantmentSlotIterator < MAX_ENCHANTMENT_SLOT; ++enchantmentSlotIterator)
            if (!item->GetEnchantmentId(EnchantmentSlot(enchantmentSlotIterator)))
                break;

        if (debugIsEnabled)
            chatHandle.PSendSysMessage(
                "{\n  iteration: %i,\n  chance: %lf,\n  threshold: %f rarity: %i,\n  tier: %i,\n  enchantment: %i,\n  exists: %i,\n  slot: %i\n}",
                i, chance, threshold, itemRarity, itemTier, enchantment, enchantmentExists ? 1 : 0, enchantmentSlotIterator);

        if (chance > threshold || itemRarity == -1 || itemTier == -1 || enchantment == -1 || !enchantmentExists || enchantmentSlotIterator >= MAX_ENCHANTMENT_SLOT)
            break;

        EnchantmentSlot enchantmentSlot = EnchantmentSlot(enchantmentSlotIterator);
        player->ApplyEnchantment(item, enchantmentSlot, false);
        item->SetEnchantment(enchantmentSlot, enchantment, 0, 0);
        player->ApplyEnchantment(item, enchantmentSlot, true);
    }

    chatHandle.PSendSysMessage("Newly Acquired |cffFF0000 %s |rhas received|cffFF0000 %i |rrandom enchantments!", itemName, i);
}

std::string getItemClassString(uint32 itemClass)
{
    if (itemClass == ITEM_CLASS_WEAPON)
        return "WEAPON";
    if (itemClass == ITEM_CLASS_ARMOR)
        return "ARMOR";
    return "";
}

int32 getRandomItemRarity(uint32 itemQuality)
{
    uint32 chance = rand_norm();
    switch (itemQuality)
    {
    case ITEM_QUALITY_POOR:
        return chance * 25;
    case ITEM_QUALITY_NORMAL:
        return chance * 50;
    case ITEM_QUALITY_UNCOMMON:
        return 45 + chance * 20;
    case ITEM_QUALITY_RARE:
        return 65 + chance * 15;
    case ITEM_QUALITY_EPIC:
        return 80 + chance * 14;
    case ITEM_QUALITY_LEGENDARY:
        return 93;
    default:
        return -1;
    }
}

int8 getItemTier(int32 itemRarity)
{
    if (itemRarity > 92)
        return 5;
    if (itemRarity > 79)
        return 4;
    if (itemRarity > 64)
        return 3;
    if (itemRarity > 44)
        return 2;
    if (itemRarity >= 0)
        return 1;
    return -1;
}

int32 getEnchantment(int32 tier, std::string classString, uint32 exclusiveSubClass)
{
    std::string query = "SELECT `enchantID`"
                        " FROM `item_enchantment_random_tiers`"
                        " WHERE `tier`={}"
                        " AND (`class`='ANY' OR `class`='{}')"
                        " AND (`exclusiveSubClass` IS NULL OR `exclusiveSubClass`={})"
                        " ORDER BY RAND()"
                        " LIMIT 1";
    QueryResult result = WorldDatabase.Query(query, tier, classString, exclusiveSubClass);

    return result ? result->Fetch()[0].Get<uint32>() : -1;
}

void RandomEnchantsPlayer::OnLogin(Player *player)
{
    if (sConfigMgr->GetOption<bool>("RandomEnchants.AnnounceOnLogin", true) && (sConfigMgr->GetOption<bool>("RandomEnchants.Enable", true)))
        ChatHandler(player->GetSession()).SendSysMessage(sConfigMgr->GetOption<std::string>("RandomEnchants.OnLoginMessage", "This server is running a RandomEnchants Module.").c_str());
}

void RandomEnchantsPlayer::OnLootItem(Player *player, Item *item, uint32 /*count*/, ObjectGuid /*lootguid*/)
{
    attemptToApplyRandomEnchantment("RandomEnchants.OnLoot", player, item);
}

void RandomEnchantsPlayer::OnCreateItem(Player *player, Item *item, uint32 /*count*/)
{
    attemptToApplyRandomEnchantment("RandomEnchants.OnCreate", player, item);
}

void RandomEnchantsPlayer::OnQuestRewardItem(Player *player, Item *item, uint32 /*count*/)
{
    attemptToApplyRandomEnchantment("RandomEnchants.OnQuestReward", player, item);
}

void RandomEnchantsPlayer::OnGroupRollRewardItem(Player *player, Item *item, uint32 /*count*/, RollVote /*voteType*/, Roll * /*roll*/)
{
    attemptToApplyRandomEnchantment("RandomEnchants.OnGroupRoll", player, item);
}
