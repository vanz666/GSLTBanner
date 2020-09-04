#pragma semicolon 1

#include <sourcemod>
#include <cstrike>
#include <gcmessages>

#pragma newdecls required

#define SEND_MSG_INTERVAL 5.0

enum
{
    k_EMsgGCCStrike15_v2_ServerVarValueNotificationInfo = 9150
}

enum
{
    CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_accountid     = 1,
    CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles    = 2,
    CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_type          = 3
}

public Plugin myinfo = 
{
    name = "GSLT Banner",
    author = "vanz",
    version = "1.0.0"
};

public void OnMapStart()
{
    CreateTimer(SEND_MSG_INTERVAL, Timer_SendMsg, _, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
}

public Action Timer_SendMsg(Handle timer)
{
    ArrayList clients = new ArrayList();

    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientConnected(i) && !IsFakeClient(i) && IsClientAuthorized(i))
            clients.Push(i);
    }

    int length = clients.Length;
    
    if (length)
    {
        int client = clients.Get(GetRandomInt(0, length - 1));
        
        int accountId = GetSteamAccountID(client);
        
        int itemId[2];
        
        int combinedDefAndPaint[2];
        combinedDefAndPaint[0] = 1; // 1 - ItemID is zero
        
        int defIdx = CS_WeaponIDToItemDefIndex(CSWeapon_KNIFE_M9_BAYONET);
        int paintKit = 12; // 12 - Crimson Web

        int combinedRank[2];
        int rank = GetRandomInt(0x0, 0x1FFFF);

        if (rank >= 0x1 && rank <= 0xFFFF)
        {
            combinedRank[0] = rank;
            combinedRank[1] = accountId | 0x80000000;
        }

        SendServerVarValueNotificationInfo(accountId, itemId, defIdx, paintKit, combinedDefAndPaint, combinedRank);
    }
    
    delete clients;
}

void SendServerVarValueNotificationInfo(int accountId, int itemId[2], int defIdx, int paintKit, int combinedDefAndPaint[2], int combinedRank[2])
{
    GCMsg_StartMessage(k_EMsgGCCStrike15_v2_ServerVarValueNotificationInfo);
    
    //GCMsg_WriteInt32(accountId, CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_accountid);
    
    GCMsg_WriteInt32(accountId,              CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    GCMsg_WriteInt32(itemId[1],              CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    GCMsg_WriteInt32(itemId[0],              CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    GCMsg_WriteInt32(defIdx,                 CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    GCMsg_WriteInt32(paintKit,               CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    GCMsg_WriteInt32(combinedDefAndPaint[1], CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    GCMsg_WriteInt32(combinedDefAndPaint[0], CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    GCMsg_WriteInt32(combinedRank[1],        CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    GCMsg_WriteInt32(combinedRank[0],        CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_viewangles);
    
    GCMsg_WriteInt32(2, CMsgGCCStrike15_v2_ServerVarValueNotificationInfo_type);

    GCMsg_EndMessage();
}