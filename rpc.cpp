// Copyright (c) 2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include "headers.h"
#undef printf
#undef snprintf
#ifdef _WIN32
#define BOOST_ASIO_DISABLE_IOCP
#endif
#include <boost/asio.hpp>
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
#define printf OutputDebugStringF
// MinGW 3.4.5 gets "fatal error: had to relocate PCH" if the json headers are
// precompiled in headers.h.  The problem might be when the pch file goes over
// a certain size around 145MB.  If we need access to json_spirit outside this
// file, we could use the compiled json_spirit option.

using boost::asio::ip::tcp;
using namespace json_spirit;

void ThreadRPCServer2(void* parg);
typedef Value(*rpcfn_type)(const Array& params, bool fHelp);
extern map<string, rpcfn_type> mapCallTable;

bool ExtractAddress(const CScript& scriptPubKey, string& addressRet)
{
    addressRet.clear();

    uint160 hash160;
    if (ExtractHash160(scriptPubKey, hash160))
    {
        addressRet = Hash160ToAddress(hash160);
        return true;
    }

    // P2PK: <pubkey> OP_CHECKSIG
    // Compressed pubkey (33 bytes): 0x21 <33 bytes> 0xac
    // Uncompressed pubkey (65 bytes): 0x41 <65 bytes> 0xac
    if (scriptPubKey.size() == 35 && scriptPubKey[0] == 33 && scriptPubKey[34] == OP_CHECKSIG)
    {
        vector<unsigned char> vchPubKey(scriptPubKey.begin() + 1, scriptPubKey.begin() + 34);
        addressRet = Hash160ToAddress(Hash160(vchPubKey));
        return true;
    }
    if (scriptPubKey.size() == 67 && scriptPubKey[0] == 65 && scriptPubKey[66] == OP_CHECKSIG)
    {
        vector<unsigned char> vchPubKey(scriptPubKey.begin() + 1, scriptPubKey.begin() + 66);
        addressRet = Hash160ToAddress(Hash160(vchPubKey));
        return true;
    }

    return false;
}





///
/// Note: This interface may still be subject to change.
///



Value help(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "help\n"
            "List commands.");

    string strRet;
    for (map<string, rpcfn_type>::iterator mi = mapCallTable.begin(); mi != mapCallTable.end(); ++mi)
    {
        try
        {
            Array params;
            (*(*mi).second)(params, true);
        }
        catch (std::exception& e)
        {
            // Help text is returned in an exception
            string strHelp = string(e.what());
            if (strHelp.find('\n') != -1)
                strHelp = strHelp.substr(0, strHelp.find('\n'));
            strRet += strHelp + "\n";
        }
    }
    strRet = strRet.substr(0,strRet.size()-1);
    return strRet;
}


Value stop(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "stop\n"
            "Stop Bitok server.");

    // Shutdown will take long enough that the response should get back
    CreateThread(Shutdown, NULL);
    return "Bitok server stopping";
}


Value getblockcount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getblockcount\n"
            "Returns the height of the most recent block in the longest block chain.");

    return nBestHeight;
}


Value getblocknumber(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getblocknumber\n"
            "Returns the block number of the latest block in the longest block chain.");

    return nBestHeight;
}


Value getblockhash(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getblockhash <index>\n"
            "Returns hash of block in best-block-chain at <index>.");

    int nHeight = params[0].get_int();
    if (nHeight < 0 || nHeight > nBestHeight)
        throw runtime_error("Block number out of range.");

    CBlockIndex* pblockindex = pindexBest;
    while (pblockindex->nHeight > nHeight)
        pblockindex = pblockindex->pprev;

    return pblockindex->GetBlockHash().ToString();
}


Value getblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getblock <hash>\n"
            "Returns information about the block with the given hash.");

    string strHash = params[0].get_str();
    uint256 hash;
    hash.SetHex(strHash);

    if (mapBlockIndex.count(hash) == 0)
        throw runtime_error("Block not found");

    CBlockIndex* pblockindex = mapBlockIndex[hash];
    CBlock block;
    block.ReadFromDisk(pblockindex, true);

    Object result;
    result.push_back(Pair("hash", block.GetHash().ToString()));
    result.push_back(Pair("version", block.nVersion));
    result.push_back(Pair("previousblockhash", block.hashPrevBlock.ToString()));
    result.push_back(Pair("merkleroot", block.hashMerkleRoot.ToString()));
    result.push_back(Pair("time", (boost::int64_t)block.nTime));
    result.push_back(Pair("bits", (boost::int64_t)block.nBits));
    result.push_back(Pair("nonce", (boost::int64_t)block.nNonce));
    result.push_back(Pair("height", pblockindex->nHeight));

    Array txhashes;
    foreach(const CTransaction& tx, block.vtx)
        txhashes.push_back(tx.GetHash().ToString());
    result.push_back(Pair("tx", txhashes));

    if (pblockindex->pnext)
        result.push_back(Pair("nextblockhash", pblockindex->pnext->GetBlockHash().ToString()));

    return result;
}


Value gettransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "gettransaction <txid>\n"
            "Get detailed information about <txid>");

    string strTxid = params[0].get_str();
    uint256 hash;
    hash.SetHex(strTxid);

    Object result;

    CRITICAL_BLOCK(cs_mapWallet)
    {
        if (mapWallet.count(hash))
        {
            const CWalletTx& wtx = mapWallet[hash];

            result.push_back(Pair("txid", hash.ToString()));
            result.push_back(Pair("version", wtx.nVersion));
            result.push_back(Pair("time", (boost::int64_t)wtx.nTimeReceived));

            int nDepth = wtx.GetDepthInMainChain();
            result.push_back(Pair("confirmations", nDepth));

            if (wtx.hashBlock != 0)
                result.push_back(Pair("blockhash", wtx.hashBlock.ToString()));

            int64 nCredit = wtx.GetCredit(true);
            int64 nDebit = wtx.GetDebit();
            int64 nNet = nCredit - nDebit;

            result.push_back(Pair("amount", (double)nNet / (double)COIN));

            if (nDebit > 0)
                result.push_back(Pair("fee", (double)(nDebit - wtx.GetValueOut()) / (double)COIN));

            Array details;
            if (nDebit > 0)
            {
                for (int i = 0; i < wtx.vout.size(); i++)
                {
                    const CTxOut& txout = wtx.vout[i];
                    if (txout.IsMine())
                        continue;

                    Object entry;
                    entry.push_back(Pair("category", "send"));

                    string strAddress;
                    if (ExtractAddress(txout.scriptPubKey, strAddress))
                        entry.push_back(Pair("address", strAddress));

                    entry.push_back(Pair("amount", (double)(-txout.nValue) / (double)COIN));
                    details.push_back(entry);
                }
            }

            if (nCredit > 0)
            {
                for (int i = 0; i < wtx.vout.size(); i++)
                {
                    const CTxOut& txout = wtx.vout[i];
                    if (!txout.IsMine())
                        continue;

                    Object entry;
                    entry.push_back(Pair("category", wtx.IsCoinBase() ? "generate" : "receive"));

                    string strAddress;
                    if (ExtractAddress(txout.scriptPubKey, strAddress))
                        entry.push_back(Pair("address", strAddress));

                    entry.push_back(Pair("amount", (double)txout.nValue / (double)COIN));
                    details.push_back(entry);
                }
            }

            result.push_back(Pair("details", details));

            return result;
        }
    }

    CTxDB txdb("r");
    CTransaction tx;
    if (txdb.ReadDiskTx(hash, tx))
    {
        result.push_back(Pair("txid", hash.ToString()));
        result.push_back(Pair("version", tx.nVersion));

        Array vin;
        foreach(const CTxIn& txin, tx.vin)
        {
            Object entry;
            if (txin.prevout.IsNull())
            {
                entry.push_back(Pair("coinbase", HexStr(txin.scriptSig.begin(), txin.scriptSig.end())));
            }
            else
            {
                entry.push_back(Pair("txid", txin.prevout.hash.ToString()));
                entry.push_back(Pair("vout", (int)txin.prevout.n));
            }
            vin.push_back(entry);
        }
        result.push_back(Pair("vin", vin));

        Array vout;
        for (int i = 0; i < tx.vout.size(); i++)
        {
            const CTxOut& txout = tx.vout[i];
            Object entry;
            entry.push_back(Pair("value", (double)txout.nValue / (double)COIN));
            entry.push_back(Pair("n", i));

            string strAddress;
            if (ExtractAddress(txout.scriptPubKey, strAddress))
                entry.push_back(Pair("address", strAddress));

            vout.push_back(entry);
        }
        result.push_back(Pair("vout", vout));

        return result;
    }

    throw runtime_error("Transaction not found");
}


Value validateaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "validateaddress <bitokaddress>\n"
            "Return information about <bitokaddress>.");

    string strAddress = params[0].get_str();

    Object result;
    result.push_back(Pair("address", strAddress));

    uint160 hash160;
    bool isValid = AddressToHash160(strAddress, hash160);
    result.push_back(Pair("isvalid", isValid));

    if (isValid)
    {
        bool isMine = mapPubKeys.count(hash160) > 0;
        result.push_back(Pair("ismine", isMine));

        if (isMine)
        {
            vector<unsigned char> vchPubKey = mapPubKeys[hash160];
            result.push_back(Pair("pubkey", HexStr(vchPubKey, false)));
        }

        CRITICAL_BLOCK(cs_mapAddressBook)
        {
            if (mapAddressBook.count(strAddress))
                result.push_back(Pair("label", mapAddressBook[strAddress]));
        }
    }

    return result;
}


Value getconnectioncount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getconnectioncount\n"
            "Returns the number of connections to other nodes.");

    return (int)vNodes.size();
}


double GetDifficulty()
{
    if (pindexBest == NULL)
        return 1.0;

    int nShift = (pindexBest->nBits >> 24) & 0xff;
    int nMantissa = pindexBest->nBits & 0x007fffff;

    if (nMantissa == 0)
        return 1.0;

    double dDiff = (double)0x007fffff / (double)nMantissa;

    while (nShift < 0x1e)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 0x1e)
    {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}

Value getdifficulty(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getdifficulty\n"
            "Returns the proof-of-work difficulty as a multiple of the minimum difficulty.");

    return GetDifficulty();
}


Value getbalance(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getbalance\n"
            "Returns the server's available balance.");

    return ((double)GetBalance() / (double)COIN);
}


Value getgenerate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getgenerate\n"
            "Returns true or false.");

    return (bool)fGenerateBitcoins;
}


Value setgenerate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "setgenerate <generate> [genproclimit]\n"
            "<generate> is true or false to turn generation on or off.\n"
            "Generation is limited to [genproclimit] processors, -1 is unlimited.");

    bool fGenerate = true;
    if (params.size() > 0)
        fGenerate = params[0].get_bool();

    if (params.size() > 1)
    {
        int nGenProcLimit = params[1].get_int();
        fLimitProcessors = (nGenProcLimit != -1);
        CWalletDB().WriteSetting("fLimitProcessors", fLimitProcessors);
        if (nGenProcLimit != -1)
            CWalletDB().WriteSetting("nLimitProcessors", nLimitProcessors = nGenProcLimit);
    }

    GenerateBitcoins(fGenerate);
    return Value::null;
}


Value getmininginfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getmininginfo\n"
            "Returns an object containing mining-related information.");

    Object obj;
    obj.push_back(Pair("blocks",            (int)nBestHeight));
    obj.push_back(Pair("currentblocksize",  (uint64_t)0));
    obj.push_back(Pair("currentblocktx",    (uint64_t)0));
    obj.push_back(Pair("difficulty",        (double)GetDifficulty()));
    obj.push_back(Pair("networkhashps",     (int64_t)GetNetworkHashPS()));
    obj.push_back(Pair("pooledtx",          (uint64_t)mapTransactions.size()));
    obj.push_back(Pair("chain",             string("main")));
    obj.push_back(Pair("generate",          (bool)fGenerateBitcoins));
    obj.push_back(Pair("genproclimit",      (int)(fLimitProcessors ? nLimitProcessors : -1)));
    return obj;
}


Value getblocktemplate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getblocktemplate [params]\n"
            "Returns data needed to construct a block to work on.\n"
            "See BIP 22 for full specification.");

    CKey key;
    key.MakeNewKey();

    CBlock* pblock = CreateNewBlock(key);
    if (!pblock)
        throw runtime_error("Out of memory");

    auto_ptr<CBlock> pblockAuto(pblock);

    Object result;
    result.push_back(Pair("version", pblock->nVersion));
    result.push_back(Pair("previousblockhash", pblock->hashPrevBlock.GetHex()));

    Array transactions;
    for (unsigned int i = 1; i < pblock->vtx.size(); i++)
    {
        const CTransaction& tx = pblock->vtx[i];
        CDataStream ssTx(SER_NETWORK);
        ssTx << tx;

        Object entry;
        entry.push_back(Pair("data", HexStr(ssTx.begin(), ssTx.end())));
        entry.push_back(Pair("txid", tx.GetHash().GetHex()));
        entry.push_back(Pair("hash", tx.GetHash().GetHex()));
        entry.push_back(Pair("depends", Array()));
        entry.push_back(Pair("fee", (int64_t)0));
        entry.push_back(Pair("sigops", (int64_t)0));
        transactions.push_back(entry);
    }
    result.push_back(Pair("transactions", transactions));

    Object coinbaseaux;
    coinbaseaux.push_back(Pair("flags", string("")));
    result.push_back(Pair("coinbaseaux", coinbaseaux));

    result.push_back(Pair("coinbasevalue", (int64_t)pblock->vtx[0].vout[0].nValue));

    uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();
    result.push_back(Pair("target", hashTarget.GetHex()));

    result.push_back(Pair("mintime", (int64_t)pblock->nTime));

    Array mutable_arr;
    mutable_arr.push_back("time");
    mutable_arr.push_back("transactions");
    mutable_arr.push_back("prevblock");
    result.push_back(Pair("mutable", mutable_arr));

    result.push_back(Pair("noncerange", string("00000000ffffffff")));
    result.push_back(Pair("sigoplimit", (int64_t)20000));
    result.push_back(Pair("sizelimit", (int64_t)MAX_BLOCK_SIZE));
    result.push_back(Pair("curtime", (int64_t)GetTime()));
    result.push_back(Pair("bits", strprintf("%08x", pblock->nBits)));
    result.push_back(Pair("height", (int64_t)(nBestHeight + 1)));

    return result;
}


Value submitblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "submitblock <hex data> [optional-params-obj]\n"
            "Attempts to submit new block to network.\n"
            "Returns null on success, error string on failure.");

    vector<unsigned char> blockData = ParseHex(params[0].get_str());
    CDataStream ssBlock(blockData, SER_NETWORK);
    CBlock* pblock = new CBlock();

    try {
        ssBlock >> *pblock;
    }
    catch (std::exception &e) {
        delete pblock;
        throw runtime_error("Block decode failed");
    }

    bool fAccepted = ProcessBlock(NULL, pblock);
    if (!fAccepted)
        return string("rejected");

    return Value::null;
}


static map<uint256, CBlock*> mapGetworkBlocks;
static map<uint256, CKey> mapGetworkKeys;
static CCriticalSection cs_getwork;

Value getwork(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getwork [data]\n"
            "If [data] is not specified, returns work data.\n"
            "If [data] is specified, tries to solve the block.");

    if (params.size() == 0)
    {
        CKey key;
        key.MakeNewKey();

        CBlock* pblock = CreateNewBlock(key);
        if (!pblock)
            throw runtime_error("Out of memory");

        unsigned char pdata[128];
        memset(pdata, 0, sizeof(pdata));

        memcpy(pdata, &pblock->nVersion, 4);
        memcpy(pdata + 4, pblock->hashPrevBlock.begin(), 32);
        memcpy(pdata + 36, pblock->hashMerkleRoot.begin(), 32);
        memcpy(pdata + 68, &pblock->nTime, 4);
        memcpy(pdata + 72, &pblock->nBits, 4);
        unsigned int nNonce = 0;
        memcpy(pdata + 76, &nNonce, 4);

        for (int i = 0; i < 32; i++)
        {
            unsigned char tmp;
            tmp = pdata[i*4];
            pdata[i*4] = pdata[i*4+3];
            pdata[i*4+3] = tmp;
            tmp = pdata[i*4+1];
            pdata[i*4+1] = pdata[i*4+2];
            pdata[i*4+2] = tmp;
        }

        uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

        uint256 hashMerkle = pblock->hashMerkleRoot;
        CRITICAL_BLOCK(cs_getwork)
        {
            mapGetworkBlocks[hashMerkle] = pblock;
            mapGetworkKeys[hashMerkle] = key;
            if (mapGetworkBlocks.size() > 100)
            {
                map<uint256, CBlock*>::iterator it = mapGetworkBlocks.begin();
                mapGetworkKeys.erase(it->first);
                delete it->second;
                mapGetworkBlocks.erase(it);
            }
        }

        string strTarget = HexStr(BEGIN(hashTarget), END(hashTarget), false);

        if (fDebug)
            printf("[getwork] bits=%08x target=%s height=%d\n",
                   pblock->nBits, strTarget.c_str(), nBestHeight + 1);

        Object result;
        result.push_back(Pair("data", HexStr(pdata, pdata + 128, false)));
        result.push_back(Pair("target", strTarget));
        result.push_back(Pair("algorithm", string("yespower")));

        return result;
    }
    else
    {
        if (fDebug)
            printf("[getwork] received submission, data length=%d\n", (int)params[0].get_str().size());

        vector<unsigned char> vchData = ParseHex(params[0].get_str());

        if (fDebug)
            printf("[getwork] parsed %d bytes\n", (int)vchData.size());

        if (vchData.size() < 80)
            throw runtime_error("Invalid parameter - data too short");

        for (size_t i = 0; i < vchData.size() / 4; i++)
        {
            unsigned char tmp;
            tmp = vchData[i*4];
            vchData[i*4] = vchData[i*4+3];
            vchData[i*4+3] = tmp;
            tmp = vchData[i*4+1];
            vchData[i*4+1] = vchData[i*4+2];
            vchData[i*4+2] = tmp;
        }

        unsigned int nVersion;
        uint256 hashPrevBlock;
        uint256 hashMerkleRoot;
        unsigned int nTime;
        unsigned int nBits;
        unsigned int nNonce;

        memcpy(&nVersion, &vchData[0], 4);
        memcpy(hashPrevBlock.begin(), &vchData[4], 32);
        memcpy(hashMerkleRoot.begin(), &vchData[36], 32);
        memcpy(&nTime, &vchData[68], 4);
        memcpy(&nBits, &vchData[72], 4);
        memcpy(&nNonce, &vchData[76], 4);

        if (fDebug)
            printf("[getwork] submit data: version=%u time=%u bits=%08x nonce=%u merkle=%s\n",
                   nVersion, nTime, nBits, nNonce, hashMerkleRoot.ToString().substr(0,16).c_str());

        CBlock* pblock = NULL;
        CKey key;
        CRITICAL_BLOCK(cs_getwork)
        {
            if (fDebug)
                printf("[getwork] mapGetworkBlocks has %d entries\n", (int)mapGetworkBlocks.size());
            if (mapGetworkBlocks.count(hashMerkleRoot))
            {
                pblock = mapGetworkBlocks[hashMerkleRoot];
                key = mapGetworkKeys[hashMerkleRoot];
            }
        }

        if (!pblock)
        {
            if (fDebug)
                printf("[getwork] ERROR: merkle root not found in map!\n");
            throw runtime_error("Stale work - block not found");
        }

        pblock->nNonce = nNonce;
        pblock->nTime = nTime;

        uint256 hash = pblock->GetPoWHash();
        uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

        if (fDebug)
            printf("[getwork] submit: nonce=%u hash=%s target=%s\n",
                   nNonce, hash.ToString().c_str(), hashTarget.ToString().c_str());

        if (hash > hashTarget)
        {
            if (fDebug)
                printf("[getwork] hash does not meet target\n");
            return false;
        }

        printf("getwork: block solved! hash=%s\n", hash.ToString().c_str());

        CRITICAL_BLOCK(cs_main)
        {
            if (pblock->hashPrevBlock != hashBestChain)
                return false;

            if (!AddKey(key))
                throw runtime_error("Failed to add key to wallet");
        }

        if (!ProcessBlock(NULL, pblock))
            throw runtime_error("Block rejected");

        CRITICAL_BLOCK(cs_getwork)
        {
            mapGetworkBlocks.erase(hashMerkleRoot);
            mapGetworkKeys.erase(hashMerkleRoot);
        }

        return true;
    }
}


Value getinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getinfo");

    Object obj;
    obj.push_back(Pair("balance",       (double)GetBalance() / (double)COIN));
    obj.push_back(Pair("blocks",        (int)nBestHeight));
    obj.push_back(Pair("connections",   (int)vNodes.size()));
    obj.push_back(Pair("proxy",         (fUseProxy ? addrProxy.ToStringIPPort() : string())));
    obj.push_back(Pair("generate",      (bool)fGenerateBitcoins));
    obj.push_back(Pair("genproclimit",  (int)(fLimitProcessors ? nLimitProcessors : -1)));
    obj.push_back(Pair("difficulty",    (double)GetDifficulty()));
    return obj;
}


Value getnewaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getnewaddress [label]\n"
            "Returns a new Bitok address for receiving payments.  "
            "If [label] is specified (recommended), it is added to the address book "
            "so payments received with the address will be labeled.");

    // Parse the label first so we don't generate a key if there's an error
    string strLabel;
    if (params.size() > 0)
        strLabel = params[0].get_str();

    // Generate a new key that is added to wallet
    string strAddress = PubKeyToAddress(GenerateNewKey());

    SetAddressBookName(strAddress, strLabel);
    return strAddress;
}


Value setlabel(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "setlabel <bitokaddress> <label>\n"
            "Sets the label associated with the given address.");

    string strAddress = params[0].get_str();
    string strLabel;
    if (params.size() > 1)
        strLabel = params[1].get_str();

    SetAddressBookName(strAddress, strLabel);
    return Value::null;
}


Value getlabel(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getlabel <bitokaddress>\n"
            "Returns the label associated with the given address.");

    string strAddress = params[0].get_str();

    string strLabel;
    CRITICAL_BLOCK(cs_mapAddressBook)
    {
        map<string, string>::iterator mi = mapAddressBook.find(strAddress);
        if (mi != mapAddressBook.end() && !(*mi).second.empty())
            strLabel = (*mi).second;
    }
    return strLabel;
}


Value getaddressesbylabel(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getaddressesbylabel <label>\n"
            "Returns the list of addresses with the given label.");

    string strLabel = params[0].get_str();

    // Find all addresses that have the given label
    Array ret;
    CRITICAL_BLOCK(cs_mapAddressBook)
    {
        foreach(const PAIRTYPE(string, string)& item, mapAddressBook)
        {
            const string& strAddress = item.first;
            const string& strName = item.second;
            if (strName == strLabel)
            {
                // We're only adding valid Bitok addresses and not ip addresses
                CScript scriptPubKey;
                if (scriptPubKey.SetBitcoinAddress(strAddress))
                    ret.push_back(strAddress);
            }
        }
    }
    return ret;
}


Value sendtoaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "sendtoaddress <bitokaddress> <amount> [comment] [comment-to]\n"
            "<amount> is a real and is rounded to the nearest 0.01");

    string strAddress = params[0].get_str();

    // Amount
    if (params[1].get_real() <= 0.0 || params[1].get_real() > 21000000.0)
        throw runtime_error("Invalid amount");
    int64 nAmount = roundint64(params[1].get_real() * 100.00) * CENT;

    // Wallet comments
    CWalletTx wtx;
    if (params.size() > 2 && params[2].type() != null_type && !params[2].get_str().empty())
        wtx.mapValue["message"] = params[2].get_str();
    if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
        wtx.mapValue["to"]      = params[3].get_str();

    string strError = SendMoneyToBitcoinAddress(strAddress, nAmount, wtx);
    if (strError != "")
        throw runtime_error(strError);
    return wtx.GetHash().ToString();
}


Value listtransactions(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "listtransactions [count=10] [includegenerated=true]\n"
            "Returns up to [count] most recent transactions.\n"
            "Returns array of objects with: txid, category, amount, confirmations, time, address");

    int64 nCount = 10;
    if (params.size() > 0)
        nCount = params[0].get_int64();
    bool fIncludeGenerated = true;
    if (params.size() > 1)
        fIncludeGenerated = params[1].get_bool();

    vector<pair<int64, uint256> > vSorted;
    CRITICAL_BLOCK(cs_mapWallet)
    {
        for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            vSorted.push_back(make_pair(wtx.nTimeReceived, wtx.GetHash()));
        }
    }
    sort(vSorted.rbegin(), vSorted.rend());

    Array ret;
    CRITICAL_BLOCK(cs_mapWallet)
    {
        for (vector<pair<int64, uint256> >::iterator it = vSorted.begin(); it != vSorted.end() && (int64)ret.size() < nCount; ++it)
        {
            map<uint256, CWalletTx>::iterator mi = mapWallet.find((*it).second);
            if (mi == mapWallet.end())
                continue;
            const CWalletTx& wtx = (*mi).second;

            bool fGenerated = wtx.IsCoinBase();
            if (fGenerated && !fIncludeGenerated)
                continue;

            int nDepth = wtx.GetDepthInMainChain();
            int64 nTime = wtx.nTimeReceived;
            string strTxid = wtx.GetHash().ToString();

            if (fGenerated)
            {
                if (nDepth < GetCoinbaseMaturity())
                    continue;
                int64 nCredit = wtx.GetCredit(true);
                Object entry;
                entry.push_back(Pair("txid", strTxid));
                entry.push_back(Pair("category", "generate"));
                entry.push_back(Pair("amount", (double)nCredit / (double)COIN));
                entry.push_back(Pair("confirmations", nDepth));
                entry.push_back(Pair("time", (boost::int64_t)nTime));
                ret.push_back(entry);
            }
            else
            {
                int64 nDebit = wtx.GetDebit();
                int64 nCredit = wtx.GetCredit(true);

                if (nDebit > 0)
                {
                    int64 nValueOut = wtx.GetValueOut();
                    int64 nFee = nDebit - nValueOut;

                    for (int i = 0; i < wtx.vout.size(); i++)
                    {
                        const CTxOut& txout = wtx.vout[i];
                        if (txout.IsMine())
                            continue;

                        string strAddress;
                        ExtractAddress(txout.scriptPubKey, strAddress);

                        Object entry;
                        entry.push_back(Pair("txid", strTxid));
                        entry.push_back(Pair("category", "send"));
                        entry.push_back(Pair("amount", (double)(-txout.nValue) / (double)COIN));
                        entry.push_back(Pair("fee", (double)(-nFee) / (double)COIN));
                        if (!strAddress.empty())
                            entry.push_back(Pair("address", strAddress));
                        entry.push_back(Pair("confirmations", nDepth));
                        entry.push_back(Pair("time", (boost::int64_t)nTime));
                        ret.push_back(entry);
                        nFee = 0;
                    }
                }

                if (nCredit > 0)
                {
                    for (int i = 0; i < wtx.vout.size(); i++)
                    {
                        const CTxOut& txout = wtx.vout[i];
                        if (!txout.IsMine())
                            continue;

                        string strAddress;
                        ExtractAddress(txout.scriptPubKey, strAddress);

                        Object entry;
                        entry.push_back(Pair("txid", strTxid));
                        entry.push_back(Pair("category", "receive"));
                        entry.push_back(Pair("amount", (double)txout.nValue / (double)COIN));
                        if (!strAddress.empty())
                            entry.push_back(Pair("address", strAddress));
                        entry.push_back(Pair("confirmations", nDepth));
                        entry.push_back(Pair("time", (boost::int64_t)nTime));
                        ret.push_back(entry);
                    }
                }
            }
        }
    }

    return ret;
}


Value getreceivedbyaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getreceivedbyaddress <bitokaddress> [minconf=1]\n"
            "Returns the total amount received by <bitokaddress> in transactions with at least [minconf] confirmations.");

    // Bitok address
    string strAddress = params[0].get_str();
    CScript scriptPubKey;
    if (!scriptPubKey.SetBitcoinAddress(strAddress))
        throw runtime_error("Invalid Bitok address");
    if (!IsMine(scriptPubKey))
        return (double)0.0;

    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    // Tally
    int64 nAmount = 0;
    CRITICAL_BLOCK(cs_mapWallet)
    {
        for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            if (wtx.IsCoinBase() || !wtx.IsFinal())
                continue;

            foreach(const CTxOut& txout, wtx.vout)
                if (txout.scriptPubKey == scriptPubKey)
                    if (wtx.GetDepthInMainChain() >= nMinDepth)
                        nAmount += txout.nValue;
        }
    }

    return (double)nAmount / (double)COIN;
}


Value getreceivedbylabel(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getreceivedbylabel <label> [minconf=1]\n"
            "Returns the total amount received by addresses with <label> in transactions with at least [minconf] confirmations.");

    // Get the set of pub keys that have the label
    string strLabel = params[0].get_str();
    set<CScript> setPubKey;
    CRITICAL_BLOCK(cs_mapAddressBook)
    {
        foreach(const PAIRTYPE(string, string)& item, mapAddressBook)
        {
            const string& strAddress = item.first;
            const string& strName = item.second;
            if (strName == strLabel)
            {
                // We're only counting our own valid Bitok addresses and not ip addresses
                CScript scriptPubKey;
                if (scriptPubKey.SetBitcoinAddress(strAddress))
                    if (IsMine(scriptPubKey))
                        setPubKey.insert(scriptPubKey);
            }
        }
    }

    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    // Tally
    int64 nAmount = 0;
    CRITICAL_BLOCK(cs_mapWallet)
    {
        for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            if (wtx.IsCoinBase() || !wtx.IsFinal())
                continue;

            foreach(const CTxOut& txout, wtx.vout)
                if (setPubKey.count(txout.scriptPubKey))
                    if (wtx.GetDepthInMainChain() >= nMinDepth)
                        nAmount += txout.nValue;
        }
    }

    return (double)nAmount / (double)COIN;
}


struct tallyitem
{
    int64 nAmount;
    int nConf;
    tallyitem()
    {
        nAmount = 0;
        nConf = INT_MAX;
    }
};

Value getbestblockhash(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getbestblockhash\n"
            "Returns the hash of the best (tip) block in the longest block chain.");

    return hashBestChain.ToString();
}


Value getrawtransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getrawtransaction <txid> [verbose=0]\n"
            "If verbose=0, returns a string that is serialized, hex-encoded data for <txid>.\n"
            "If verbose is non-zero, returns an Object with information about <txid>.");

    string strTxid = params[0].get_str();
    uint256 hash;
    hash.SetHex(strTxid);

    bool fVerbose = false;
    if (params.size() > 1)
        fVerbose = (params[1].get_int() != 0);

    CTransaction tx;
    uint256 hashBlock = 0;
    CTxIndex txindex;

    CRITICAL_BLOCK(cs_mapTransactions)
    {
        if (mapTransactions.count(hash))
        {
            tx = mapTransactions[hash];
        }
    }

    if (tx.IsNull())
    {
        CTxDB txdb("r");
        if (!txdb.ReadDiskTx(hash, tx, txindex))
            throw runtime_error("No information available about transaction");
    }

    CDataStream ssTx;
    ssTx << tx;
    string strHex = HexStr(ssTx.begin(), ssTx.end());

    if (!fVerbose)
        return strHex;

    Object result;
    result.push_back(Pair("hex", strHex));
    result.push_back(Pair("txid", hash.ToString()));
    result.push_back(Pair("version", tx.nVersion));
    result.push_back(Pair("locktime", (boost::int64_t)tx.nLockTime));

    Array vin;
    foreach(const CTxIn& txin, tx.vin)
    {
        Object entry;
        if (txin.prevout.IsNull())
        {
            entry.push_back(Pair("coinbase", HexStr(txin.scriptSig.begin(), txin.scriptSig.end())));
        }
        else
        {
            entry.push_back(Pair("txid", txin.prevout.hash.ToString()));
            entry.push_back(Pair("vout", (int)txin.prevout.n));
            entry.push_back(Pair("scriptSig", HexStr(txin.scriptSig.begin(), txin.scriptSig.end())));
        }
        entry.push_back(Pair("sequence", (boost::int64_t)txin.nSequence));
        vin.push_back(entry);
    }
    result.push_back(Pair("vin", vin));

    Array vout;
    for (int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];
        Object entry;
        entry.push_back(Pair("value", (double)txout.nValue / (double)COIN));
        entry.push_back(Pair("n", i));
        entry.push_back(Pair("scriptPubKey", HexStr(txout.scriptPubKey.begin(), txout.scriptPubKey.end(), false)));

        string strAddress;
        if (ExtractAddress(txout.scriptPubKey, strAddress))
            entry.push_back(Pair("address", strAddress));

        vout.push_back(entry);
    }
    result.push_back(Pair("vout", vout));

    return result;
}


Value getrawmempool(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getrawmempool\n"
            "Returns all transaction ids in memory pool.");

    Array ret;
    CRITICAL_BLOCK(cs_mapTransactions)
    {
        for (map<uint256, CTransaction>::iterator mi = mapTransactions.begin();
             mi != mapTransactions.end(); ++mi)
        {
            ret.push_back((*mi).first.ToString());
        }
    }
    return ret;
}


Value listunspent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "listunspent [minconf=1] [maxconf=9999999]\n"
            "Returns array of unspent transaction outputs\n"
            "with between minconf and maxconf (inclusive) confirmations.");

    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    int nMaxDepth = 9999999;
    if (params.size() > 1)
        nMaxDepth = params[1].get_int();

    Array results;
    CTxDB txdb("r");

    CRITICAL_BLOCK(cs_mapWallet)
    {
        for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;

            if (wtx.IsCoinBase() && wtx.GetBlocksToMaturity() > 0)
                continue;

            int nDepth = wtx.GetDepthInMainChain();
            if (nDepth < nMinDepth || nDepth > nMaxDepth)
                continue;

            for (int i = 0; i < wtx.vout.size(); i++)
            {
                const CTxOut& txout = wtx.vout[i];

                if (!txout.IsMine())
                    continue;

                CTxIndex txindex;
                if (!txdb.ReadTxIndex(wtx.GetHash(), txindex))
                    continue;

                if (i < txindex.vSpent.size() && !txindex.vSpent[i].IsNull())
                    continue;

                Object entry;
                entry.push_back(Pair("txid", wtx.GetHash().ToString()));
                entry.push_back(Pair("vout", i));

                string strAddress;
                if (ExtractAddress(txout.scriptPubKey, strAddress))
                    entry.push_back(Pair("address", strAddress));

                entry.push_back(Pair("scriptPubKey", HexStr(txout.scriptPubKey.begin(), txout.scriptPubKey.end(), false)));
                entry.push_back(Pair("amount", (double)txout.nValue / (double)COIN));
                entry.push_back(Pair("confirmations", nDepth));

                results.push_back(entry);
            }
        }
    }

    return results;
}


Value getpeerinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getpeerinfo\n"
            "Returns data about each connected network node.");

    Array ret;
    CRITICAL_BLOCK(cs_vNodes)
    {
        foreach(CNode* pnode, vNodes)
        {
            Object obj;
            obj.push_back(Pair("addr", pnode->addr.ToString()));
            obj.push_back(Pair("services", strprintf("%08" PRI64x, pnode->nServices)));
            obj.push_back(Pair("lastsend", (boost::int64_t)pnode->nLastSend));
            obj.push_back(Pair("lastrecv", (boost::int64_t)pnode->nLastRecv));
            obj.push_back(Pair("conntime", (boost::int64_t)pnode->nTimeConnected));
            obj.push_back(Pair("version", pnode->nVersion));
            obj.push_back(Pair("inbound", pnode->fInbound));
            obj.push_back(Pair("startingheight", pnode->nStartingHeight));
            ret.push_back(obj);
        }
    }
    return ret;
}


Value ListReceived(const Array& params, bool fByLabels)
{
    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    // Whether to include empty accounts
    bool fIncludeEmpty = false;
    if (params.size() > 1)
        fIncludeEmpty = params[1].get_bool();

    // Tally
    map<uint160, tallyitem> mapTally;
    CRITICAL_BLOCK(cs_mapWallet)
    {
        for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            if (wtx.IsCoinBase() || !wtx.IsFinal())
                continue;

            int nDepth = wtx.GetDepthInMainChain();
            if (nDepth < nMinDepth)
                continue;

            foreach(const CTxOut& txout, wtx.vout)
            {
                // Only counting our own Bitok addresses and not ip addresses
                uint160 hash160 = txout.scriptPubKey.GetBitcoinAddressHash160();
                if (hash160 == 0 || !mapPubKeys.count(hash160)) // IsMine
                    continue;

                tallyitem& item = mapTally[hash160];
                item.nAmount += txout.nValue;
                item.nConf = min(item.nConf, nDepth);
            }
        }
    }

    // Reply
    Array ret;
    map<string, tallyitem> mapLabelTally;
    CRITICAL_BLOCK(cs_mapAddressBook)
    {
        foreach(const PAIRTYPE(string, string)& item, mapAddressBook)
        {
            const string& strAddress = item.first;
            const string& strLabel = item.second;
            uint160 hash160;
            if (!AddressToHash160(strAddress, hash160))
                continue;
            map<uint160, tallyitem>::iterator it = mapTally.find(hash160);
            if (it == mapTally.end() && !fIncludeEmpty)
                continue;

            int64 nAmount = 0;
            int nConf = INT_MAX;
            if (it != mapTally.end())
            {
                nAmount = (*it).second.nAmount;
                nConf = (*it).second.nConf;
            }

            if (fByLabels)
            {
                tallyitem& item = mapLabelTally[strLabel];
                item.nAmount += nAmount;
                item.nConf = min(item.nConf, nConf);
            }
            else
            {
                Object obj;
                obj.push_back(Pair("address",       strAddress));
                obj.push_back(Pair("label",         strLabel));
                obj.push_back(Pair("amount",        (double)nAmount / (double)COIN));
                obj.push_back(Pair("confirmations", (nConf == INT_MAX ? 0 : nConf)));
                ret.push_back(obj);
            }
        }
    }

    if (fByLabels)
    {
        for (map<string, tallyitem>::iterator it = mapLabelTally.begin(); it != mapLabelTally.end(); ++it)
        {
            int64 nAmount = (*it).second.nAmount;
            int nConf = (*it).second.nConf;
            Object obj;
            obj.push_back(Pair("label",         (*it).first));
            obj.push_back(Pair("amount",        (double)nAmount / (double)COIN));
            obj.push_back(Pair("confirmations", (nConf == INT_MAX ? 0 : nConf)));
            ret.push_back(obj);
        }
    }

    return ret;
}

Value listreceivedbyaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "listreceivedbyaddress [minconf=1] [includeempty=false]\n"
            "[minconf] is the minimum number of confirmations before payments are included.\n"
            "[includeempty] whether to include addresses that haven't received any payments.\n"
            "Returns an array of objects containing:\n"
            "  \"address\" : receiving address\n"
            "  \"label\" : the label of the receiving address\n"
            "  \"amount\" : total amount received by the address\n"
            "  \"confirmations\" : number of confirmations of the most recent transaction included");

    return ListReceived(params, false);
}

Value listreceivedbylabel(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "listreceivedbylabel [minconf=1] [includeempty=false]\n"
            "[minconf] is the minimum number of confirmations before payments are included.\n"
            "[includeempty] whether to include labels that haven't received any payments.\n"
            "Returns an array of objects containing:\n"
            "  \"label\" : the label of the receiving addresses\n"
            "  \"amount\" : total amount received by addresses with this label\n"
            "  \"confirmations\" : number of confirmations of the most recent transaction included");

    return ListReceived(params, true);
}













//
// Call Table
//

pair<string, rpcfn_type> pCallTable[] =
{
    make_pair("help",                  &help),
    make_pair("stop",                  &stop),
    make_pair("getblockcount",         &getblockcount),
    make_pair("getblocknumber",        &getblocknumber),
    make_pair("getblockhash",          &getblockhash),
    make_pair("getbestblockhash",      &getbestblockhash),
    make_pair("getblock",              &getblock),
    make_pair("gettransaction",        &gettransaction),
    make_pair("getrawtransaction",     &getrawtransaction),
    make_pair("getrawmempool",         &getrawmempool),
    make_pair("validateaddress",       &validateaddress),
    make_pair("getconnectioncount",    &getconnectioncount),
    make_pair("getpeerinfo",           &getpeerinfo),
    make_pair("getdifficulty",         &getdifficulty),
    make_pair("getbalance",            &getbalance),
    make_pair("getgenerate",           &getgenerate),
    make_pair("setgenerate",           &setgenerate),
    make_pair("getmininginfo",         &getmininginfo),
    make_pair("getblocktemplate",      &getblocktemplate),
    make_pair("submitblock",           &submitblock),
    make_pair("getwork",               &getwork),
    make_pair("getinfo",               &getinfo),
    make_pair("getnewaddress",         &getnewaddress),
    make_pair("setlabel",              &setlabel),
    make_pair("getlabel",              &getlabel),
    make_pair("getaddressesbylabel",   &getaddressesbylabel),
    make_pair("sendtoaddress",         &sendtoaddress),
    make_pair("listtransactions",      &listtransactions),
    make_pair("listunspent",           &listunspent),
    make_pair("getamountreceived",     &getreceivedbyaddress), // deprecated, renamed to getreceivedbyaddress
    make_pair("getallreceived",        &listreceivedbyaddress), // deprecated, renamed to listreceivedbyaddress
    make_pair("getreceivedbyaddress",  &getreceivedbyaddress),
    make_pair("getreceivedbylabel",    &getreceivedbylabel),
    make_pair("listreceivedbyaddress", &listreceivedbyaddress),
    make_pair("listreceivedbylabel",   &listreceivedbylabel),
};
map<string, rpcfn_type> mapCallTable(pCallTable, pCallTable + sizeof(pCallTable)/sizeof(pCallTable[0]));




//
// HTTP protocol
//
// This ain't Apache.  We're just using HTTP header for the length field
// and to be compatible with other JSON-RPC implementations.
//

string EncodeBase64(const string& str);

string HTTPPost(const string& strMsg)
{
    string strUserPass = GetArg("-rpcuser", "") + ":" + GetArg("-rpcpassword", "");
    string strAuth;
    if (strUserPass != ":")
        strAuth = strprintf("Authorization: Basic %s\r\n", EncodeBase64(strUserPass).c_str());

    return strprintf(
            "POST / HTTP/1.1\r\n"
            "User-Agent: json-rpc/1.0\r\n"
            "Host: 127.0.0.1\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Accept: application/json\r\n"
            "%s"
            "\r\n"
            "%s",
        strMsg.size(),
        strAuth.c_str(),
        strMsg.c_str());
}

string HTTPReply(const string& strMsg, int nStatus=200)
{
    string strStatus;
    if (nStatus == 200) strStatus = "OK";
    if (nStatus == 401) strStatus = "Unauthorized";
    if (nStatus == 403) strStatus = "Forbidden";
    if (nStatus == 500) strStatus = "Internal Server Error";

    string strHeaders = strprintf(
            "HTTP/1.1 %d %s\r\n"
            "Connection: close\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: application/json\r\n"
            "Date: Sat, 08 Jul 2006 12:04:08 GMT\r\n"
            "Server: json-rpc/1.0\r\n",
        nStatus,
        strStatus.c_str(),
        strMsg.size());

    if (nStatus == 401)
        strHeaders += "WWW-Authenticate: Basic realm=\"jsonrpc\"\r\n";

    return strHeaders + "\r\n" + strMsg;
}

string EncodeBase64(const string& str)
{
    const char* pbase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string result;
    int val = 0;
    int valb = -6;
    for (unsigned char c : str)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            result.push_back(pbase64[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        result.push_back(pbase64[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4)
        result.push_back('=');
    return result;
}

string DecodeBase64(const string& str)
{
    const char* pbase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string result;
    int val = 0;
    int valb = -8;
    for (unsigned char c : str)
    {
        if (c == '=') break;
        const char* p = strchr(pbase64, c);
        if (p == NULL) continue;
        val = (val << 6) + (p - pbase64);
        valb += 6;
        if (valb >= 0)
        {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return result;
}

bool HTTPAuthorized(const string& strAuth)
{
    string strRPCUserColonPass = GetArg("-rpcuser", "") + ":" + GetArg("-rpcpassword", "");
    if (strRPCUserColonPass == ":")
        return true;

    if (strAuth.substr(0, 6) != "Basic ")
        return false;

    string strUserPass64 = strAuth.substr(6);
    string strUserPass = DecodeBase64(strUserPass64);
    return strUserPass == strRPCUserColonPass;
}

bool ClientAllowed(const string& strAddr)
{
    if (!mapMultiArgs.count("-rpcallowip"))
        return strAddr == "127.0.0.1";

    foreach(string strAllow, mapMultiArgs["-rpcallowip"])
    {
        if (strAllow == strAddr)
            return true;

        size_t pos = strAllow.find('/');
        if (pos != string::npos)
        {
            unsigned int mask = 32;
            if (pos < strAllow.length() - 1)
                mask = atoi(strAllow.substr(pos + 1).c_str());

            string strNet = strAllow.substr(0, pos);
            unsigned int ip1[4], ip2[4];
            if (sscanf(strAddr.c_str(), "%u.%u.%u.%u", &ip1[0], &ip1[1], &ip1[2], &ip1[3]) == 4 &&
                sscanf(strNet.c_str(), "%u.%u.%u.%u", &ip2[0], &ip2[1], &ip2[2], &ip2[3]) == 4)
            {
                unsigned int nIP1 = (ip1[0] << 24) | (ip1[1] << 16) | (ip1[2] << 8) | ip1[3];
                unsigned int nIP2 = (ip2[0] << 24) | (ip2[1] << 16) | (ip2[2] << 8) | ip2[3];
                unsigned int nMask = 0xFFFFFFFF << (32 - mask);

                if ((nIP1 & nMask) == (nIP2 & nMask))
                    return true;
            }
        }
    }
    return false;
}

int ReadHTTPHeader(tcp::iostream& stream, map<string, string>& mapHeadersRet)
{
    int nLen = 0;
    loop
    {
        string str;
        std::getline(stream, str);
        if (str.empty() || str == "\r")
            break;
        if (str.substr(0,15) == "Content-Length:")
            nLen = atoi(str.substr(15));
        else
        {
            string::size_type nColon = str.find(":");
            if (nColon != string::npos)
            {
                string strHeader = str.substr(0, nColon);
                string strValue = str.substr(nColon + 1);
                while (!strValue.empty() && isspace(strValue[0]))
                    strValue = strValue.substr(1);
                while (!strValue.empty() && (isspace(strValue[strValue.size()-1]) || strValue[strValue.size()-1] == '\r'))
                    strValue = strValue.substr(0, strValue.size()-1);
                mapHeadersRet[strHeader] = strValue;
            }
        }
    }
    return nLen;
}

inline string ReadHTTP(tcp::iostream& stream, map<string, string>& mapHeadersRet)
{
    mapHeadersRet.clear();
    int nLen = ReadHTTPHeader(stream, mapHeadersRet);
    if (nLen <= 0)
        return string();

    vector<char> vch(nLen);
    stream.read(&vch[0], nLen);
    return string(vch.begin(), vch.end());
}



//
// JSON-RPC protocol
//
// http://json-rpc.org/wiki/specification
// http://www.codeproject.com/KB/recipes/JSON_Spirit.aspx
//

string JSONRPCRequest(const string& strMethod, const Array& params, const Value& id)
{
    Object request;
    request.push_back(Pair("method", strMethod));
    request.push_back(Pair("params", params));
    request.push_back(Pair("id", id));
    return write_string(Value(request), false) + "\n";
}

string JSONRPCReply(const Value& result, const Value& error, const Value& id)
{
    Object reply;
    if (error.type() != null_type)
        reply.push_back(Pair("result", Value::null));
    else
        reply.push_back(Pair("result", result));
    reply.push_back(Pair("error", error));
    reply.push_back(Pair("id", id));
    return write_string(Value(reply), false) + "\n";
}




void ThreadRPCServer(void* parg)
{
    IMPLEMENT_RANDOMIZE_STACK(ThreadRPCServer(parg));
    try
    {
        vnThreadsRunning[4]++;
        ThreadRPCServer2(parg);
        vnThreadsRunning[4]--;
    }
    catch (std::exception& e) {
        vnThreadsRunning[4]--;
        PrintException(&e, "ThreadRPCServer()");
    } catch (...) {
        vnThreadsRunning[4]--;
        PrintException(NULL, "ThreadRPCServer()");
    }
    printf("ThreadRPCServer exiting\n");
}

void ThreadRPCServer2(void* parg)
{
    printf("ThreadRPCServer started\n");

    int nRPCPort = GetIntArg("-rpcport", 8332);
    string strRPCBind = GetArg("-rpcbind", "127.0.0.1");

    boost::asio::ip::address bindAddress;
    try
    {
#if BOOST_VERSION >= 106600
        bindAddress = boost::asio::ip::make_address(strRPCBind);
#else
        bindAddress = boost::asio::ip::address::from_string(strRPCBind);
#endif
    }
    catch (...)
    {
        printf("Invalid -rpcbind address: %s, using 127.0.0.1\n", strRPCBind.c_str());
        bindAddress = boost::asio::ip::address_v4::loopback();
    }

    printf("RPC server binding to %s:%d\n", bindAddress.to_string().c_str(), nRPCPort);

#if BOOST_VERSION >= 106600
    boost::asio::io_context io_service;
#else
    boost::asio::io_service io_service;
#endif
    tcp::endpoint endpoint(bindAddress, nRPCPort);
    tcp::acceptor acceptor(io_service, endpoint);

    loop
    {
        tcp::iostream stream;
        tcp::endpoint peer;
        vnThreadsRunning[4]--;
#if BOOST_VERSION >= 106600
        acceptor.accept(stream.rdbuf()->socket(), peer);
#else
        acceptor.accept(*stream.rdbuf(), peer);
#endif
        vnThreadsRunning[4]++;
        if (fShutdown)
            return;

        string strPeerAddr = peer.address().to_string();

        if (!ClientAllowed(strPeerAddr))
        {
            printf("RPC connection from %s denied\n", strPeerAddr.c_str());
            stream << HTTPReply("Forbidden", 403) << std::flush;
            continue;
        }

        map<string, string> mapHeaders;
        string strRequest = ReadHTTP(stream, mapHeaders);

        if (!HTTPAuthorized(mapHeaders["Authorization"]))
        {
            printf("RPC authorization failed from %s\n", strPeerAddr.c_str());
            stream << HTTPReply("Unauthorized", 401) << std::flush;
            continue;
        }

        if (fDebug)
            printf("[RPC] Request from %s\n", strPeerAddr.c_str());

        // Handle multiple invocations per request
        string::iterator begin = strRequest.begin();
        while (skipspaces(begin), begin != strRequest.end())
        {
            string::iterator prev = begin;
            Value id;
            try
            {
                // Parse request
                Value valRequest;
                if (!read_range(begin, strRequest.end(), valRequest))
                    throw runtime_error("Parse error.");
                const Object& request = valRequest.get_obj();
                if (find_value(request, "method").type() != str_type ||
                    find_value(request, "params").type() != array_type)
                    throw runtime_error("Invalid request.");

                string strMethod    = find_value(request, "method").get_str();
                const Array& params = find_value(request, "params").get_array();
                id                  = find_value(request, "id");

                // Execute
                map<string, rpcfn_type>::iterator mi = mapCallTable.find(strMethod);
                if (mi == mapCallTable.end())
                    throw runtime_error("Method not found.");

                int64 nStartTime = GetTimeMillis();
                Value result = (*(*mi).second)(params, false);
                int64 nDuration = GetTimeMillis() - nStartTime;

                if (fDebug)
                    printf("[RPC] %s (%d params) completed in %lld ms\n", strMethod.c_str(), (int)params.size(), (long long)nDuration);

                // Send reply
                string strReply = JSONRPCReply(result, Value::null, id);
                stream << HTTPReply(strReply, 200) << std::flush;
            }
            catch (std::exception& e)
            {
                // Send error reply
                string strReply = JSONRPCReply(Value::null, e.what(), id);
                stream << HTTPReply(strReply, 500) << std::flush;
            }
            if (begin == prev)
                break;
        }
    }
}




Value CallRPC(const string& strMethod, const Array& params)
{
    int nRPCPort = GetIntArg("-rpcport", 8332);
    string strRPCHost = GetArg("-rpcconnect", "127.0.0.1");

    tcp::iostream stream(strRPCHost, itostr(nRPCPort));
    if (stream.fail())
        throw runtime_error("couldn't connect to server");

    string strRequest = JSONRPCRequest(strMethod, params, 1);
    stream << HTTPPost(strRequest) << std::flush;

    map<string, string> mapHeaders;
    string strReply = ReadHTTP(stream, mapHeaders);
    if (strReply.empty())
        throw runtime_error("no response from server");

    // Parse reply
    Value valReply;
    if (!read_string(strReply, valReply))
        throw runtime_error("couldn't parse reply from server");
    const Object& reply = valReply.get_obj();
    if (reply.empty())
        throw runtime_error("expected reply to have result, error and id properties");

    const Value& result = find_value(reply, "result");
    const Value& error  = find_value(reply, "error");
    const Value& id     = find_value(reply, "id");

    if (error.type() == str_type)
        throw runtime_error(error.get_str());
    else if (error.type() != null_type)
        throw runtime_error(write_string(error, false));
    return result;
}




template<typename T>
void ConvertTo(Value& value)
{
    if (value.type() == str_type)
    {
        // reinterpret string as unquoted json value
        Value value2;
        if (!read_string(value.get_str(), value2))
            throw runtime_error("type mismatch");
        value = value2.get_value<T>();
    }
    else
    {
        value = value.get_value<T>();
    }
}

int CommandLineRPC(int argc, char *argv[])
{
    try
    {
        // Check that method exists
        if (argc < 2)
            throw runtime_error("too few parameters");
        string strMethod = argv[1];
        if (!mapCallTable.count(strMethod))
            throw runtime_error(strprintf("unknown command: %s", strMethod.c_str()));

        Value result;
        if (argc == 3 && (strcmp(argv[2], "-?") == 0 || strcmp(argv[2], "--help") == 0))
        {
            // Call help locally, help text is returned in an exception
            try
            {
                map<string, rpcfn_type>::iterator mi = mapCallTable.find(strMethod);
                Array params;
                (*(*mi).second)(params, true);
            }
            catch (std::exception& e)
            {
                result = e.what();
            }
        }
        else
        {
            // Parameters default to strings
            Array params;
            for (int i = 2; i < argc; i++)
                params.push_back(argv[i]);
            int n = params.size();

            //
            // Special case non-string parameter types
            //
            if (strMethod == "getblockhash"           && n > 0) ConvertTo<boost::int64_t>(params[0]);
            if (strMethod == "getrawtransaction"      && n > 1) ConvertTo<boost::int64_t>(params[1]);
            if (strMethod == "setgenerate"            && n > 0) ConvertTo<bool>(params[0]);
            if (strMethod == "setgenerate"            && n > 1) ConvertTo<boost::int64_t>(params[1]);
            if (strMethod == "sendtoaddress"          && n > 1) ConvertTo<double>(params[1]);
            if (strMethod == "listtransactions"       && n > 0) ConvertTo<boost::int64_t>(params[0]);
            if (strMethod == "listtransactions"       && n > 1) ConvertTo<bool>(params[1]);
            if (strMethod == "listunspent"            && n > 0) ConvertTo<boost::int64_t>(params[0]);
            if (strMethod == "listunspent"            && n > 1) ConvertTo<boost::int64_t>(params[1]);
            if (strMethod == "getamountreceived"      && n > 1) ConvertTo<boost::int64_t>(params[1]); // deprecated
            if (strMethod == "getreceivedbyaddress"   && n > 1) ConvertTo<boost::int64_t>(params[1]);
            if (strMethod == "getreceivedbylabel"     && n > 1) ConvertTo<boost::int64_t>(params[1]);
            if (strMethod == "getallreceived"         && n > 0) ConvertTo<boost::int64_t>(params[0]); // deprecated
            if (strMethod == "getallreceived"         && n > 1) ConvertTo<bool>(params[1]);
            if (strMethod == "listreceivedbyaddress"  && n > 0) ConvertTo<boost::int64_t>(params[0]);
            if (strMethod == "listreceivedbyaddress"  && n > 1) ConvertTo<bool>(params[1]);
            if (strMethod == "listreceivedbylabel"    && n > 0) ConvertTo<boost::int64_t>(params[0]);
            if (strMethod == "listreceivedbylabel"    && n > 1) ConvertTo<bool>(params[1]);

            // Execute
            result = CallRPC(strMethod, params);
        }

        // Print result
        string strResult = (result.type() == str_type ? result.get_str() : write_string(result, true));
        if (result.type() != null_type)
        {
            if (fWindows && fGUI)
                // Windows GUI apps can't print to command line,
                // so settle for a message box yuck
                MyMessageBox(strResult.c_str(), "Bitok", wxOK);
            else
                fprintf(stdout, "%s\n", strResult.c_str());
        }
        return 0;
    }
    catch (std::exception& e) {
        if (fWindows && fGUI)
            MyMessageBox(strprintf("error: %s\n", e.what()).c_str(), "Bitok", wxOK);
        else
            fprintf(stderr, "error: %s\n", e.what());
    } catch (...) {
        PrintException(NULL, "CommandLineRPC()");
    }
    return 1;
}




#ifdef TEST
int main(int argc, char *argv[])
{
#ifdef _MSC_VER
    // Turn off microsoft heap dump noise
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, CreateFile("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    try
    {
        if (argc >= 2 && string(argv[1]) == "-server")
        {
            printf("server ready\n");
            ThreadRPCServer(NULL);
        }
        else
        {
            return CommandLineRPC(argc, argv);
        }
    }
    catch (std::exception& e) {
        PrintException(&e, "main()");
    } catch (...) {
        PrintException(NULL, "main()");
    }
    return 0;
}
#endif
