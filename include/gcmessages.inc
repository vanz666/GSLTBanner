#if defined _gcmessages_included
 #endinput
#endif
#define _gcmessages_included

#include <steamworks>

#define GCMSG_BUF_SIZE 8192
#define GCMSG_PROTO_MASK 0x80000000

enum
{
    ProtoWireType_Varint = 0,
    ProtoWireType_64bit,
    ProtoWireType_LenDelim,
    ProtoWireType_StartGroup,
    ProtoWireType_EndGroup,
    ProtoWireType_32bit
}

static char g_msgBuf[GCMSG_BUF_SIZE];

static int g_msgPos = -1;

stock void GCMsg_StartMessage(int msgType)
{
    if (g_msgPos != -1)
    {
        LogError("GC message already started.");
        return;
    }

    msgType |= GCMSG_PROTO_MASK;
    
    g_msgBuf[0] = msgType;
    g_msgBuf[1] = msgType >> 8;
    g_msgBuf[2] = msgType >> 16;
    g_msgBuf[3] = msgType >> 24;
    
    g_msgPos = 8;
}

stock void GCMsg_EndMessage()
{
    if (g_msgPos == -1)
    {
        LogError("GC message not started.");
        return;
    }

    int msgType = g_msgBuf[0] | (g_msgBuf[1] << 8) | (g_msgBuf[2] << 16) | (g_msgBuf[3] << 24);
    
    EGCResults result = SteamWorks_SendMessageToGC(msgType, g_msgBuf, g_msgPos);
    
    if (result != k_EGCResultOK)
        LogError("Failed to send GC message (%d)", result);
        
    g_msgPos = -1;
}

stock void GCMsg_WriteString(const char[] data, int field_number)
{
    if (g_msgPos == -1)
    {
        LogError("GC message not started.");
        return;
    }

    g_msgBuf[g_msgPos++] = (field_number << 3) | ProtoWireType_LenDelim;
    
    int strLen = strlen(data);
    
    WriteVarInt32(strLen);
    
    strcopy(g_msgBuf[g_msgPos], sizeof(g_msgBuf) - g_msgPos, data);
    
    g_msgPos += strLen;
}

stock void GCMsg_WriteInt32(int data, int field_number)
{
    if (g_msgPos == -1)
    {
        LogError("GC message not started.");
        return;
    }

    g_msgBuf[g_msgPos++] = (field_number << 3) | ProtoWireType_Varint;
    
    WriteVarInt32(data);
}

stock void GCMsg_WriteInt64(const int data[2], int field_number)
{
    if (g_msgPos == -1)
    {
        LogError("GC message not started.");
        return;
    }

    g_msgBuf[g_msgPos++] = (field_number << 3) | ProtoWireType_Varint;
    
    WriteVarInt64(data);
}

stock void GCMsg_WriteSInt32(int data, int field_number)
{
    GCMsg_WriteInt32(EncodeZigZag32(data), field_number);
}

stock void GCMsg_WriteSInt64(const int data[2], int field_number)
{
    int encoded[2];
    encoded = data;
    EncodeZigZag64(encoded);
    GCMsg_WriteInt64(encoded, field_number);
}

stock void GCMsg_WriteFixed32(int data, int field_number)
{
    if (g_msgPos == -1)
    {
        LogError("GC message not started.");
        return;
    }

    g_msgBuf[g_msgPos++] = (field_number << 3) | ProtoWireType_32bit;
    
    g_msgBuf[g_msgPos++] = data;
    g_msgBuf[g_msgPos++] = data >> 8;
    g_msgBuf[g_msgPos++] = data >> 16;
    g_msgBuf[g_msgPos++] = data >> 24;
}

stock void GCMsg_WriteFixed64(const int data[2], int field_number)
{
    if (g_msgPos == -1)
    {
        LogError("GC message not started.");
        return;
    }

    g_msgBuf[g_msgPos++] = (field_number << 3) | ProtoWireType_64bit;
    
    g_msgBuf[g_msgPos++] = data[0];
    g_msgBuf[g_msgPos++] = data[0] >> 8;
    g_msgBuf[g_msgPos++] = data[0] >> 16;
    g_msgBuf[g_msgPos++] = data[0] >> 24;
    g_msgBuf[g_msgPos++] = data[1];
    g_msgBuf[g_msgPos++] = data[1] >> 8;
    g_msgBuf[g_msgPos++] = data[1] >> 16;
    g_msgBuf[g_msgPos++] = data[1] >> 24;
}

stock void GCMsg_WriteFloat(float data, int field_number)
{
    GCMsg_WriteFixed32(view_as<int>(data), field_number);
}

stock void GCMsg_WriteBool(bool data, int field_number)
{
    GCMsg_WriteInt32(data, field_number);
}

static void WriteVarInt32(int data)
{
    g_msgBuf[g_msgPos] = data | 0x80;
    
    if ((data ^ 0x80000000) >= ((1 << 7) ^ 0x80000000))
    {
        g_msgBuf[g_msgPos + 1] = (data >>> 7) | 0x80;
        
        if ((data ^ 0x80000000) >= ((1 << 14) ^ 0x80000000))
        {
            g_msgBuf[g_msgPos + 2] = (data >>> 14) | 0x80;
            
            if ((data ^ 0x80000000) >= ((1 << 21) ^ 0x80000000))
            {
                g_msgBuf[g_msgPos + 3] = (data >>> 21) | 0x80;
                
                if ((data ^ 0x80000000) >= ((1 << 28) ^ 0x80000000))
                {
                    g_msgBuf[g_msgPos + 4] = data >>> 28;
                    g_msgPos += 5;
                }
                else
                {
                    g_msgBuf[g_msgPos + 3] &= 0x7F;
                    g_msgPos += 4;
                }
            }
            else
            {
                g_msgBuf[g_msgPos + 2] &= 0x7F;
                g_msgPos += 3;
            }
        }
        else
        {
            g_msgBuf[g_msgPos + 1] &= 0x7F;
            g_msgPos += 2;
        }
    }
    else
    {
        g_msgBuf[g_msgPos++] &= 0x7F;
    }
}

static void WriteVarInt64(const int data[2])
{
    int split[3];
    
    split[0] =  data[0];
    split[1] = (data[1] << 4) | (data[0] >>> 28);
    split[2] =  data[1] >>> 24;
    
    int size;

    if (split[2] == 0)
    {
        if (split[1] == 0)
        {
            if ((split[0] ^ 0x80000000) < ((1 << 14) ^ 0x80000000))
            {
                if ((split[0] ^ 0x80000000) < ((1 << 7) ^ 0x80000000))
                    size = 1;
                else
                    size = 2;
            }
            else
            {
                if ((split[0] ^ 0x80000000) < ((1 << 21) ^ 0x80000000))
                    size = 3;
                else
                    size = 4;
            }
        }
        else
        {
            if ((split[1] ^ 0x80000000) < ((1 << 14) ^ 0x80000000))
            {
                if ((split[1] ^ 0x80000000) < ((1 << 7) ^ 0x80000000))
                    size = 5;
                else
                    size = 6;
            }
            else
            {
                if ((split[1] ^ 0x80000000) < ((1 << 21) ^ 0x80000000))
                    size = 7;
                else
                    size = 8;
            }
        }
    }
    else
    {
        if ((split[2] ^ 0x80000000) < ((1 << 7) ^ 0x80000000))
            size = 9;
        else
            size = 10;
    }
    
    for (int i = size - 1; i >= 0; i--)
        g_msgBuf[g_msgPos + i] = (split[i / 4] >>> ((i * 7) % 28)) | 0x80;
    
    g_msgBuf[g_msgPos + (size - 1)] &= 0x7F;
    
    g_msgPos += size;
}

static int EncodeZigZag32(int data)
{
    return (data << 1) ^ (data >> 31);
}

static void EncodeZigZag64(int data[2])
{
    int a = data[0] >>> 31;
    int b = data[1] >> 31;
    
    data[0] =  (data[0] << 1)        ^ b;
    data[1] = ((data[1] << 1) | a)    ^ b;
}