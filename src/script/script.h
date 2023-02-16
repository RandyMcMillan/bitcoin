// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SCRIPT_SCRIPT_H
#define BITCOIN_SCRIPT_SCRIPT_H

#include <attributes.h>
#include <crypto/common.h>
#include <prevector.h>
#include <serialize.h>

#include <assert.h>
#include <climits>
#include <limits>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

// Maximum number of bytes pushable to the stack
static const unsigned int MAX_SCRIPT_ELEMENT_SIZE = 520;

// Maximum number of non-push operations per script
static const int MAX_OPS_PER_SCRIPT = 201;

// Maximum number of public keys per multisig
static const int MAX_PUBKEYS_PER_MULTISIG = 20;

/** The limit of keys in OP_CHECKSIGADD-based scripts. It is due to the stack limit in BIP342. */
static constexpr unsigned int MAX_PUBKEYS_PER_MULTI_A = 999;

// Maximum script length in bytes
static const int MAX_SCRIPT_SIZE = 10000;

// Maximum number of values on script interpreter stack
static const int MAX_STACK_SIZE = 1000;

// Threshold for nLockTime: below this value it is interpreted as block number,
// otherwise as UNIX timestamp.
static const unsigned int LOCKTIME_THRESHOLD = 500000000; // Tue Nov  5 00:53:20 1985 UTC

// Maximum nLockTime. Since a lock time indicates the last invalid timestamp, a
// transaction with this lock time will never be valid unless lock time
// checking is disabled (by setting all input sequence numbers to
// SEQUENCE_FINAL).
static const uint32_t LOCKTIME_MAX = 0xFFFFFFFFU;

// Tag for input annex. If there are at least two witness elements for a transaction input,
// and the first byte of the last element is 0x50, this last element is called annex, and
// has meanings independent of the script
static constexpr unsigned int ANNEX_TAG = 0x50;

// Validation weight per passing signature (Tapscript only, see BIP 342).
static constexpr int64_t VALIDATION_WEIGHT_PER_SIGOP_PASSED{50};

// How much weight budget is added to the witness size (Tapscript only, see BIP 342).
static constexpr int64_t VALIDATION_WEIGHT_OFFSET{50};

template <typename T>
std::vector<unsigned char> ToByteVector(const T& in)
{
    return std::vector<unsigned char>(in.begin(), in.end());
}

/** Script opcodes */
enum opcodetype
{
    // push value
    OP_0 = 0x00, // push "" onto the stack (which is an empty array of bytes)
    OP_FALSE = OP_0, // push "" onto the stack (which is an empty array of bytes)
    OP_PUSHDATA1 = 0x4c, // read the next byte as N and push the next N bytes as an array onto the stack
    OP_PUSHDATA2 = 0x4d, // read the next 2 bytes as N and push the next N bytes as an array onto the stack
    OP_PUSHDATA4 = 0x4e, // // read the next 4 bytes as N and push the next N bytes as an array onto the stack
    OP_1NEGATE = 0x4f, // push "\x81" onto the stack (which is interpreted as -1 by numerical opcodes)
    OP_RESERVED = 0x50, // mark tx invalid unless occurring in an unexecuted OP_IF branch
    OP_1 = 0x51, // push "\x01" onto the stack (which is interpreted as 1 by numerical opcodes)
    OP_TRUE=OP_1, // push "\x01" onto the stack (which is interpreted as 1 by numerical opcodes)
    OP_2 = 0x52, // push "\x02" onto the stack (which is interpreted as 2 by numerical opcodes)
    OP_3 = 0x53, // push "\x03" onto the stack (which is interpreted as 3 by numerical opcodes)
    OP_4 = 0x54, // push "\x04" onto the stack (which is interpreted as 4 by numerical opcodes)
    OP_5 = 0x55, // push "\x05" onto the stack (which is interpreted as 5 by numerical opcodes)
    OP_6 = 0x56, // push "\x06" onto the stack (which is interpreted as 6 by numerical opcodes)
    OP_7 = 0x57, // push "\x07" onto the stack (which is interpreted as 7 by numerical opcodes)
    OP_8 = 0x58, // push "\x08" onto the stack (which is interpreted as 8 by numerical opcodes)
    OP_9 = 0x59, // push "\x09" onto the stack (which is interpreted as 9 by numerical opcodes)
    OP_10 = 0x5a, // push "\x0A" onto the stack (which is interpreted as 10 by numerical opcodes)
    OP_11 = 0x5b, // push "\x0B" onto the stack (which is interpreted as 11 by numerical opcodes)
    OP_12 = 0x5c, // push "\x0C" onto the stack (which is interpreted as 12 by numerical opcodes)
    OP_13 = 0x5d, // push "\x0D" onto the stack (which is interpreted as 13 by numerical opcodes)
    OP_14 = 0x5e, // push "\x0E" onto the stack (which is interpreted as 14 by numerical opcodes)
    OP_15 = 0x5f, // push "\x0F" onto the stack (which is interpreted as 15 by numerical opcodes)
    OP_16 = 0x60, // push "\x10" onto the stack (which is interpreted as 16 by numerical opcodes)

    // control
    OP_NOP = 0x61, // do nothing
    OP_VER = 0x62, // mark tx invalid unless occuring in an unexecuted OP_IF branch
    OP_IF = 0x63, // if top stack value true, execute the statement
    OP_NOTIF = 0x64, // if top stack value false, execute the statement
    OP_VERIF = 0x65, // mark tx invalid even when occuring in an unexecuted OP_IF branch
    OP_VERNOTIF = 0x66, // mark tx invalid even when occuring in an unexecuted OP_IF branch
    OP_ELSE = 0x67, // if the preceding OP_IF, OP_NOTIF or OP_ELSE not executed, execute the statement
    OP_ENDIF = 0x68, // end if/else block (must include, otherwise tx becomes invalid)
    OP_VERIFY = 0x69, // mark tx invalid if top stack value false, unless occuring in an unexecuted OP_IF branch
    OP_RETURN = 0x6a, // mark tx invalid unless occuring in an unexecuted OP_IF branch

    // stack ops
    OP_TOALTSTACK = 0x6b, // pop an item from the main stack onto the alt stack
    OP_FROMALTSTACK = 0x6c, // pop an item from the alt stack onto the main stack
    OP_2DROP = 0x6d, // remove the two top stack items
    OP_2DUP = 0x6e, // duplicate top and second from top stack items
    OP_3DUP = 0x6f, // duplicate top, second from top and third from top stack items
    OP_2OVER = 0x70, // copy third and fourth from top stack items to the top
    OP_2ROT = 0x71, // move fifth and sixth from top stack items to the top
    OP_2SWAP = 0x72, // swap: top and second from top <-> third and fourth from top items of stack
    OP_IFDUP = 0x73, // if top stack value != 0, duplicate it
    OP_DEPTH = 0x74, // push the current number of stack items onto the stack
    OP_DROP = 0x75, // remove top stack item
    OP_DUP = 0x76, // duplicate top stack item
    OP_NIP = 0x77, // remove second from top stack item
    OP_OVER = 0x78, // copy second from top stack item to the top
    OP_PICK = 0x79, // copy item N back in stack to the top
    OP_ROLL = 0x7a, // move item N back in stack
    OP_ROT = 0x7b, // move third from top stack item to the top
    OP_SWAP = 0x7c, // swap top two items of stack
    OP_TUCK = 0x7d, // copy top stack item and insert before second from top item

    // splice ops
    OP_CAT = 0x7e, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_SUBSTR = 0x7f, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_LEFT = 0x80, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_RIGHT = 0x81, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_SIZE = 0x82, // push the length of top stack item (not pop the top element whose size is inspected)

    // bit logic
    OP_INVERT = 0x83, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_AND = 0x84, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_OR = 0x85, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_XOR = 0x86, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_EQUAL = 0x87, // return 1 if the inputs are equal, 0 otherwise
    OP_EQUALVERIFY = 0x88, // execute OP_EQUAL, then OP_VERIFY afterward
    OP_RESERVED1 = 0x89, // mark tx invalid unless occuring in an unexecuted OP_IF branch
    OP_RESERVED2 = 0x8a, // mark tx invalid unless occuring in an unexecuted OP_IF branch

    // numeric
    OP_1ADD = 0x8b, // 1 added to the top stack item
    OP_1SUB = 0x8c, // 1 subtracted from the top stack item
    OP_2MUL = 0x8d, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_2DIV = 0x8e, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_NEGATE = 0x8f, // multiply the top stack item by -1
    OP_ABS = 0x90, // absolute the top stack item value
    OP_NOT = 0x91, // convert 0 to 1 and else to 0
    OP_0NOTEQUAL = 0x92, // return 0 if the input is 0, otherwise 1

    OP_ADD = 0x93, // pop top two stack items and push their sum
    OP_SUB = 0x94, // pop top two stack items and push the second minus the top
    OP_MUL = 0x95, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_DIV = 0x96, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_MOD = 0x97, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_LSHIFT = 0x98, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)
    OP_RSHIFT = 0x99, // disabled, fail the script unconditionally (apply in an unexecuted conditional branch)

    OP_BOOLAND = 0x9a, // return 1 if both top two stack items are not 0, otherwise 0
    OP_BOOLOR = 0x9b, // return 1 if neither top nor second from top stack item is 0, otherwise 0
    OP_NUMEQUAL = 0x9c, // return 1 if inputs are equal, otherwise 0
    OP_NUMEQUALVERIFY = 0x9d, // execute OP_NUMEQUAL, then OP_VERIFY afterward
    OP_NUMNOTEQUAL = 0x9e, // return 1 if inputs are not equal, otherwise 0
    OP_LESSTHAN = 0x9f, // pop top two stack items and push 1 if second from top < top, otherwise 0
    OP_GREATERTHAN = 0xa0, // pop top two stack items and push 1 if second from top > top, otherwise 0
    OP_LESSTHANOREQUAL = 0xa1, // pop top two stack items and push 1 if second from top <= top, otherwise 0
    OP_GREATERTHANOREQUAL = 0xa2, // pop top two stack items and push 1 if second from top >= top, otherwise 0
    OP_MIN = 0xa3, // pop the top two items and push the smaller
    OP_MAX = 0xa4, // pop the top two items and push the bigger

    OP_WITHIN = 0xa5, // pop the top three items and push 1 if third from top > top >= second from top, otherwise 0

    // crypto
    OP_RIPEMD160 = 0xa6, // hash input using RIPEMD-160
    OP_SHA1 = 0xa7, // hash input using SHA-1
    OP_SHA256 = 0xa8, // hash input using SHA-256
    OP_HASH160 = 0xa9, // hash input using SHA-256, then RIPEMD-160
    OP_HASH256 = 0xaa, // hash input using SHA-256 twice
    OP_CODESEPARATOR = 0xab, // ignore this and everything preceding, when determine what to sign when signature-checking
    OP_CHECKSIG = 0xac, // push 1 if signature is valid for tx hash and public key, otherwise 0
    OP_CHECKSIGVERIFY = 0xad, // execute OP_CHECKSIG, then OP_VERIFY afterward
    OP_CHECKMULTISIG = 0xae, // OP_CHECKSIG for muti-signature
    OP_CHECKMULTISIGVERIFY = 0xaf, // execute OP_CHECKMULTISIG, then OP_VERIFY afterward

    // expansion
    OP_NOP1 = 0xb0, // do nothing
    OP_CHECKLOCKTIMEVERIFY = 0xb1, // mark tx invalid if top stack item > nLockTimefield of tx, otherwise script evaluation continues as if OP_NOP was executed
    OP_NOP2 = OP_CHECKLOCKTIMEVERIFY, // mark tx invalid if top stack item > nLockTimefield of tx, otherwise script evaluation continues as if OP_NOP was executed
    OP_CHECKSEQUENCEVERIFY = 0xb2, // mark tx invalid if the relative lock time of the input is not equal to or longer than the value of the top stack item
    OP_NOP3 = OP_CHECKSEQUENCEVERIFY, // mark tx invalid if the relative lock time of the input is not equal to or longer than the value of the top stack item
    OP_NOP4 = 0xb3, // do nothing
    OP_NOP5 = 0xb4, // do nothing
    OP_NOP6 = 0xb5, // do nothing
    OP_NOP7 = 0xb6, // do nothing
    OP_NOP8 = 0xb7, // do nothing
    OP_NOP9 = 0xb8, // do nothing
    OP_NOP10 = 0xb9, // do nothing

    // Opcode added by BIP 342 (Tapscript)
    OP_CHECKSIGADD = 0xba, // mark tx invalid

    OP_INVALIDOPCODE = 0xff, // match any opcode not yet assigned
};

// Maximum value that an opcode can be
static const unsigned int MAX_OPCODE = OP_NOP10;

std::string GetOpName(opcodetype opcode);

class scriptnum_error : public std::runtime_error
{
public:
    explicit scriptnum_error(const std::string& str) : std::runtime_error(str) {}
};

class CScriptNum
{
/**
 * Numeric opcodes (OP_1ADD, etc) are restricted to operating on 4-byte integers.
 * The semantics are subtle, though: operands must be in the range [-2^31 +1...2^31 -1],
 * but results may overflow (and are valid as long as they are not used in a subsequent
 * numeric operation). CScriptNum enforces those semantics by storing results as
 * an int64 and allowing out-of-range values to be returned as a vector of bytes but
 * throwing an exception if arithmetic is done or the result is interpreted as an integer.
 */
public:

    explicit CScriptNum(const int64_t& n)
    {
        m_value = n;
    }

    static const size_t nDefaultMaxNumSize = 4;

    explicit CScriptNum(const std::vector<unsigned char>& vch, bool fRequireMinimal,
                        const size_t nMaxNumSize = nDefaultMaxNumSize)
    {
        if (vch.size() > nMaxNumSize) {
            throw scriptnum_error("script number overflow");
        }
        if (fRequireMinimal && vch.size() > 0) {
            // Check that the number is encoded with the minimum possible
            // number of bytes.
            //
            // If the most-significant-byte - excluding the sign bit - is zero
            // then we're not minimal. Note how this test also rejects the
            // negative-zero encoding, 0x80.
            if ((vch.back() & 0x7f) == 0) {
                // One exception: if there's more than one byte and the most
                // significant bit of the second-most-significant-byte is set
                // it would conflict with the sign bit. An example of this case
                // is +-255, which encode to 0xff00 and 0xff80 respectively.
                // (big-endian).
                if (vch.size() <= 1 || (vch[vch.size() - 2] & 0x80) == 0) {
                    throw scriptnum_error("non-minimally encoded script number");
                }
            }
        }
        m_value = set_vch(vch);
    }

    inline bool operator==(const int64_t& rhs) const    { return m_value == rhs; }
    inline bool operator!=(const int64_t& rhs) const    { return m_value != rhs; }
    inline bool operator<=(const int64_t& rhs) const    { return m_value <= rhs; }
    inline bool operator< (const int64_t& rhs) const    { return m_value <  rhs; }
    inline bool operator>=(const int64_t& rhs) const    { return m_value >= rhs; }
    inline bool operator> (const int64_t& rhs) const    { return m_value >  rhs; }

    inline bool operator==(const CScriptNum& rhs) const { return operator==(rhs.m_value); }
    inline bool operator!=(const CScriptNum& rhs) const { return operator!=(rhs.m_value); }
    inline bool operator<=(const CScriptNum& rhs) const { return operator<=(rhs.m_value); }
    inline bool operator< (const CScriptNum& rhs) const { return operator< (rhs.m_value); }
    inline bool operator>=(const CScriptNum& rhs) const { return operator>=(rhs.m_value); }
    inline bool operator> (const CScriptNum& rhs) const { return operator> (rhs.m_value); }

    inline CScriptNum operator+(   const int64_t& rhs)    const { return CScriptNum(m_value + rhs);}
    inline CScriptNum operator-(   const int64_t& rhs)    const { return CScriptNum(m_value - rhs);}
    inline CScriptNum operator+(   const CScriptNum& rhs) const { return operator+(rhs.m_value);   }
    inline CScriptNum operator-(   const CScriptNum& rhs) const { return operator-(rhs.m_value);   }

    inline CScriptNum& operator+=( const CScriptNum& rhs)       { return operator+=(rhs.m_value);  }
    inline CScriptNum& operator-=( const CScriptNum& rhs)       { return operator-=(rhs.m_value);  }

    inline CScriptNum operator&(   const int64_t& rhs)    const { return CScriptNum(m_value & rhs);}
    inline CScriptNum operator&(   const CScriptNum& rhs) const { return operator&(rhs.m_value);   }

    inline CScriptNum& operator&=( const CScriptNum& rhs)       { return operator&=(rhs.m_value);  }

    inline CScriptNum operator-()                         const
    {
        assert(m_value != std::numeric_limits<int64_t>::min());
        return CScriptNum(-m_value);
    }

    inline CScriptNum& operator=( const int64_t& rhs)
    {
        m_value = rhs;
        return *this;
    }

    inline CScriptNum& operator+=( const int64_t& rhs)
    {
        assert(rhs == 0 || (rhs > 0 && m_value <= std::numeric_limits<int64_t>::max() - rhs) ||
                           (rhs < 0 && m_value >= std::numeric_limits<int64_t>::min() - rhs));
        m_value += rhs;
        return *this;
    }

    inline CScriptNum& operator-=( const int64_t& rhs)
    {
        assert(rhs == 0 || (rhs > 0 && m_value >= std::numeric_limits<int64_t>::min() + rhs) ||
                           (rhs < 0 && m_value <= std::numeric_limits<int64_t>::max() + rhs));
        m_value -= rhs;
        return *this;
    }

    inline CScriptNum& operator&=( const int64_t& rhs)
    {
        m_value &= rhs;
        return *this;
    }

    int getint() const
    {
        if (m_value > std::numeric_limits<int>::max())
            return std::numeric_limits<int>::max();
        else if (m_value < std::numeric_limits<int>::min())
            return std::numeric_limits<int>::min();
        return m_value;
    }

    int64_t GetInt64() const { return m_value; }

    std::vector<unsigned char> getvch() const
    {
        return serialize(m_value);
    }

    static std::vector<unsigned char> serialize(const int64_t& value)
    {
        if(value == 0)
            return std::vector<unsigned char>();

        std::vector<unsigned char> result;
        const bool neg = value < 0;
        uint64_t absvalue = neg ? ~static_cast<uint64_t>(value) + 1 : static_cast<uint64_t>(value);

        while(absvalue)
        {
            result.push_back(absvalue & 0xff);
            absvalue >>= 8;
        }

//    - If the most significant byte is >= 0x80 and the value is positive, push a
//    new zero-byte to make the significant byte < 0x80 again.

//    - If the most significant byte is >= 0x80 and the value is negative, push a
//    new 0x80 byte that will be popped off when converting to an integral.

//    - If the most significant byte is < 0x80 and the value is negative, add
//    0x80 to it, since it will be subtracted and interpreted as a negative when
//    converting to an integral.

        if (result.back() & 0x80)
            result.push_back(neg ? 0x80 : 0);
        else if (neg)
            result.back() |= 0x80;

        return result;
    }

private:
    static int64_t set_vch(const std::vector<unsigned char>& vch)
    {
      if (vch.empty())
          return 0;

      int64_t result = 0;
      for (size_t i = 0; i != vch.size(); ++i)
          result |= static_cast<int64_t>(vch[i]) << 8*i;

      // If the input vector's most significant byte is 0x80, remove it from
      // the result's msb and return a negative.
      if (vch.back() & 0x80)
          return -((int64_t)(result & ~(0x80ULL << (8 * (vch.size() - 1)))));

      return result;
    }

    int64_t m_value;
};

/**
 * We use a prevector for the script to reduce the considerable memory overhead
 *  of vectors in cases where they normally contain a small number of small elements.
 * Tests in October 2015 showed use of this reduced dbcache memory usage by 23%
 *  and made an initial sync 13% faster.
 */
typedef prevector<28, unsigned char> CScriptBase;

bool GetScriptOp(CScriptBase::const_iterator& pc, CScriptBase::const_iterator end, opcodetype& opcodeRet, std::vector<unsigned char>* pvchRet);

/** Serialized script, used inside transaction inputs and outputs */
class CScript : public CScriptBase
{
protected:
    CScript& push_int64(int64_t n)
    {
        if (n == -1 || (n >= 1 && n <= 16))
        {
            push_back(n + (OP_1 - 1));
        }
        else if (n == 0)
        {
            push_back(OP_0);
        }
        else
        {
            *this << CScriptNum::serialize(n);
        }
        return *this;
    }
public:
    CScript() { }
    CScript(const_iterator pbegin, const_iterator pend) : CScriptBase(pbegin, pend) { }
    CScript(std::vector<unsigned char>::const_iterator pbegin, std::vector<unsigned char>::const_iterator pend) : CScriptBase(pbegin, pend) { }
    CScript(const unsigned char* pbegin, const unsigned char* pend) : CScriptBase(pbegin, pend) { }

    SERIALIZE_METHODS(CScript, obj) { READWRITEAS(CScriptBase, obj); }

    explicit CScript(int64_t b) { operator<<(b); }
    explicit CScript(opcodetype b)     { operator<<(b); }
    explicit CScript(const CScriptNum& b) { operator<<(b); }
    // delete non-existent constructor to defend against future introduction
    // e.g. via prevector
    explicit CScript(const std::vector<unsigned char>& b) = delete;

    /** Delete non-existent operator to defend against future introduction */
    CScript& operator<<(const CScript& b) = delete;

    CScript& operator<<(int64_t b) LIFETIMEBOUND { return push_int64(b); }

    CScript& operator<<(opcodetype opcode) LIFETIMEBOUND
    {
        if (opcode < 0 || opcode > 0xff)
            throw std::runtime_error("CScript::operator<<(): invalid opcode");
        insert(end(), (unsigned char)opcode);
        return *this;
    }

    CScript& operator<<(const CScriptNum& b) LIFETIMEBOUND
    {
        *this << b.getvch();
        return *this;
    }

    CScript& operator<<(const std::vector<unsigned char>& b) LIFETIMEBOUND
    {
        if (b.size() < OP_PUSHDATA1)
        {
            insert(end(), (unsigned char)b.size());
        }
        else if (b.size() <= 0xff)
        {
            insert(end(), OP_PUSHDATA1);
            insert(end(), (unsigned char)b.size());
        }
        else if (b.size() <= 0xffff)
        {
            insert(end(), OP_PUSHDATA2);
            uint8_t _data[2];
            WriteLE16(_data, b.size());
            insert(end(), _data, _data + sizeof(_data));
        }
        else
        {
            insert(end(), OP_PUSHDATA4);
            uint8_t _data[4];
            WriteLE32(_data, b.size());
            insert(end(), _data, _data + sizeof(_data));
        }
        insert(end(), b.begin(), b.end());
        return *this;
    }

    bool GetOp(const_iterator& pc, opcodetype& opcodeRet, std::vector<unsigned char>& vchRet) const
    {
        return GetScriptOp(pc, end(), opcodeRet, &vchRet);
    }

    bool GetOp(const_iterator& pc, opcodetype& opcodeRet) const
    {
        return GetScriptOp(pc, end(), opcodeRet, nullptr);
    }

    /** Encode/decode small integers: */
    static int DecodeOP_N(opcodetype opcode)
    {
        if (opcode == OP_0)
            return 0;
        assert(opcode >= OP_1 && opcode <= OP_16);
        return (int)opcode - (int)(OP_1 - 1);
    }
    static opcodetype EncodeOP_N(int n)
    {
        assert(n >= 0 && n <= 16);
        if (n == 0)
            return OP_0;
        return (opcodetype)(OP_1+n-1);
    }

    /**
     * Pre-version-0.6, Bitcoin always counted CHECKMULTISIGs
     * as 20 sigops. With pay-to-script-hash, that changed:
     * CHECKMULTISIGs serialized in scriptSigs are
     * counted more accurately, assuming they are of the form
     *  ... OP_N CHECKMULTISIG ...
     */
    unsigned int GetSigOpCount(bool fAccurate) const;

    /**
     * Accurately count sigOps, including sigOps in
     * pay-to-script-hash transactions:
     */
    unsigned int GetSigOpCount(const CScript& scriptSig) const;

    bool IsPayToScriptHash() const;
    bool IsPayToWitnessScriptHash() const;
    bool IsWitnessProgram(int& version, std::vector<unsigned char>& program) const;

    /** Called by IsStandardTx and P2SH/BIP62 VerifyScript (which makes it consensus-critical). */
    bool IsPushOnly(const_iterator pc) const;
    bool IsPushOnly() const;

    /** Check if the script contains valid OP_CODES */
    bool HasValidOps() const;

    /**
     * Returns whether the script is guaranteed to fail at execution,
     * regardless of the initial stack. This allows outputs to be pruned
     * instantly when entering the UTXO set.
     */
    bool IsUnspendable() const
    {
        return (size() > 0 && *begin() == OP_RETURN) || (size() > MAX_SCRIPT_SIZE);
    }

    void clear()
    {
        // The default prevector::clear() does not release memory
        CScriptBase::clear();
        shrink_to_fit();
    }
};

struct CScriptWitness
{
    // Note that this encodes the data elements being pushed, rather than
    // encoding them as a CScript that pushes them.
    std::vector<std::vector<unsigned char> > stack;

    // Some compilers complain without a default constructor
    CScriptWitness() { }

    bool IsNull() const { return stack.empty(); }

    void SetNull() { stack.clear(); stack.shrink_to_fit(); }

    std::string ToString() const;
};

/** Test for OP_SUCCESSx opcodes as defined by BIP342. */
bool IsOpSuccess(const opcodetype& opcode);

bool CheckMinimalPush(const std::vector<unsigned char>& data, opcodetype opcode);

/** Build a script by concatenating other scripts, or any argument accepted by CScript::operator<<. */
template<typename... Ts>
CScript BuildScript(Ts&&... inputs)
{
    CScript ret;
    int cnt{0};

    ([&ret, &cnt] (Ts&& input) {
        if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<Ts>>, CScript>) {
            // If it is a CScript, extend ret with it. Move or copy the first element instead.
            if (cnt == 0) {
                ret = std::forward<Ts>(input);
            } else {
                ret.insert(ret.end(), input.begin(), input.end());
            }
        } else {
            // Otherwise invoke CScript::operator<<.
            ret << input;
        }
        cnt++;
    } (std::forward<Ts>(inputs)), ...);

    return ret;
}

#endif // BITCOIN_SCRIPT_SCRIPT_H
