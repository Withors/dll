#pragma once

#include <string>
#include <unordered_map>

#include "../Actor/Actor.hpp"
#include "../../../Utils/Memory/Game/SignatureAndOffsetManager.hpp"

class Actor;

class mcUUID {
public:
    uint64_t mostSig, leastSig;
};

enum PlayerListPacketType : int8_t {
    AddPlayerEntry = 0x0,  // The names are just add and removed but i dont want to cause issues
    RemovePlayerEntry = 0x1,
};

class PlayerListEntry {
public:
    uint64_t id; // This is the ActorUniqueID
    mcUUID UUID;
    std::string name, XUID, platformOnlineId;
    int buildPlatform;
    char filler[0x164];
    //SerializedSkin skin;
    bool isTeacher, isHost;
};

class Level {
    using troll = std::unordered_map<mcUUID, PlayerListEntry>;

public:
    troll &getPlayerMap() {
        return direct_access<troll>(this, 0x1E98);
    }

    std::vector<Actor *> getRuntimeActorList() {
        // TODO prevent crashing !!!
        static uintptr_t sig = Memory::findSig(GET_SIG("Level::getRuntimeActorList"));
        static auto getRuntimeActorList = *(decltype(&Level::getRuntimeActorList) *) &sig;
        return (this->*getRuntimeActorList)();
    }
};