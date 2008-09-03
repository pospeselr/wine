/*
 * Unit test suite for crypt32.dll's Crypt*Message functions
 *
 * Copyright 2007-2008 Juan Lang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <winerror.h>
#include <wincrypt.h>

#include "wine/test.h"

static const BYTE dataEmptyBareContent[] = { 0x04,0x00 };
static const BYTE dataEmptyContent[] = {
0x30,0x0f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x02,
0x04,0x00 };
static const BYTE signedEmptyBareContent[] = {
0x30,0x50,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0x31,0x37,0x30,0x35,0x02,
0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,
0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,
0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,
0x04,0x06,0x00,0x05,0x00,0x04,0x00 };
static const BYTE signedEmptyContent[] = {
0x30,0x5f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,0x52,
0x30,0x50,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,
0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0x31,0x37,0x30,0x35,0x02,
0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,
0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,
0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,
0x04,0x06,0x00,0x05,0x00,0x04,0x00 };

static void test_msg_get_signer_count(void)
{
    LONG count;

    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(0, NULL, 0);
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == E_INVALIDARG, "Expected E_INVALIDARG, got %08x\n",
     GetLastError());
    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING, NULL, 0);
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == CRYPT_E_ASN1_EOD ||
       GetLastError() == OSS_BAD_ARG, /* win9x */
     "Expected CRYPT_E_ASN1_EOD, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING,
     dataEmptyBareContent, sizeof(dataEmptyBareContent));
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == CRYPT_E_ASN1_BADTAG ||
       GetLastError() == OSS_PDU_MISMATCH, /* win9x */
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING,
     dataEmptyContent, sizeof(dataEmptyContent));
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == CRYPT_E_INVALID_MSG_TYPE,
     "Expected CRYPT_E_INVALID_MSG_TYPE, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING,
     signedEmptyBareContent, sizeof(signedEmptyBareContent));
    ok(count == -1, "Expected -1, got %d\n", count);
    ok(GetLastError() == CRYPT_E_ASN1_BADTAG ||
       GetLastError() == OSS_DATA_ERROR, /* win9x */
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    count = CryptGetMessageSignerCount(PKCS_7_ASN_ENCODING,
     signedEmptyContent, sizeof(signedEmptyContent));
    ok(count == 1 ||
       broken(count == -1), /* win9x */
       "Expected 1, got %d\n", count);
}

static BYTE detachedHashContent[] = {
0x30,0x3f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x05,0xa0,0x32,
0x30,0x30,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x0b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0x04,0x10,0x08,0xd6,0xc0,0x5a,0x21,0x51,0x2a,0x79,0xa1,0xdf,0xeb,
0x9d,0x2a,0x8f,0x26,0x2f };
static const BYTE msgData[] = { 1, 2, 3, 4 };

static void test_verify_detached_message_hash(void)
{
    BOOL ret;
    CRYPT_HASH_MESSAGE_PARA para;
    DWORD size, hashSize;
    const BYTE *pMsgData = msgData;
    BYTE hash[16];

    if (0)
    {
        ret = CryptVerifyDetachedMessageHash(NULL, NULL, 0, 0, NULL, NULL, NULL,
         NULL);
    }
    memset(&para, 0, sizeof(para));
    SetLastError(0xdeadbeef);
    ret = CryptVerifyDetachedMessageHash(&para, NULL, 0, 0, NULL, NULL, NULL,
     NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "expected E_INVALIDARG, got %08x\n", GetLastError());
    para.cbSize = sizeof(para);
    SetLastError(0xdeadbeef);
    ret = CryptVerifyDetachedMessageHash(&para, NULL, 0, 0, NULL, NULL, NULL,
     NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "expected E_INVALIDARG, got %08x\n", GetLastError());
    para.dwMsgEncodingType = PKCS_7_ASN_ENCODING;
    SetLastError(0xdeadbeef);
    ret = CryptVerifyDetachedMessageHash(&para, NULL, 0, 0, NULL, NULL, NULL,
     NULL);
    ok(!ret &&
     (GetLastError() == CRYPT_E_ASN1_EOD ||
      GetLastError() == OSS_BAD_ARG), /* win9x */
     "expected CRYPT_E_ASN1_EOD, got %08x\n", GetLastError());
    para.dwMsgEncodingType = X509_ASN_ENCODING;
    SetLastError(0xdeadbeef);
    ret = CryptVerifyDetachedMessageHash(&para, NULL, 0, 0, NULL, NULL, NULL,
     NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "expected E_INVALIDARG, got %08x\n", GetLastError());
    para.dwMsgEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    SetLastError(0xdeadbeef);
    ret = CryptVerifyDetachedMessageHash(&para, NULL, 0, 0, NULL, NULL, NULL,
     NULL);
    ok(!ret &&
     (GetLastError() == CRYPT_E_ASN1_EOD ||
      GetLastError() == OSS_BAD_ARG), /* win9x */
     "expected CRYPT_E_ASN1_EOD, got %08x\n", GetLastError());
    /* Curiously, passing no data to hash succeeds.. */
    ret = CryptVerifyDetachedMessageHash(&para, detachedHashContent,
     sizeof(detachedHashContent), 0, NULL, NULL, NULL, NULL);
    todo_wine
    ok(ret, "CryptVerifyDetachedMessageHash failed: %08x\n", GetLastError());
    /* as does passing the actual content of the message to hash.. */
    size = sizeof(msgData);
    pMsgData = msgData;
    ret = CryptVerifyDetachedMessageHash(&para, detachedHashContent,
     sizeof(detachedHashContent), 1, &pMsgData, &size, NULL, NULL);
    ok(ret, "CryptVerifyDetachedMessageHash failed: %08x\n", GetLastError());
    /* while passing data to hash that isn't the content of the message fails.
     */
    size = sizeof(detachedHashContent);
    pMsgData = detachedHashContent;
    SetLastError(0xdeadbeef);
    ret = CryptVerifyDetachedMessageHash(&para, detachedHashContent,
     sizeof(detachedHashContent), 1, &pMsgData, &size, NULL, NULL);
    ok(!ret && GetLastError() == CRYPT_E_HASH_VALUE,
     "expected CRYPT_E_HASH_VALUE, got %08x\n", GetLastError());
    /* Getting the size of the hash while passing no hash data causes the
     * hash to be checked (and fail.)
     */
    SetLastError(0xdeadbeef);
    ret = CryptVerifyDetachedMessageHash(&para, detachedHashContent,
     sizeof(detachedHashContent), 0, NULL, NULL, NULL, &hashSize);
    ok(!ret && GetLastError() == CRYPT_E_HASH_VALUE,
     "expected CRYPT_E_HASH_VALUE, got %08x\n", GetLastError());
    size = sizeof(msgData);
    pMsgData = msgData;
    ret = CryptVerifyDetachedMessageHash(&para, detachedHashContent,
     sizeof(detachedHashContent), 1, &pMsgData, &size, NULL, &hashSize);
    ok(ret, "CryptVerifyDetachedMessageHash failed: %08x\n", GetLastError());
    ok(hashSize == sizeof(hash), "unexpected size %d\n", hashSize);
    hashSize = 1;
    SetLastError(0xdeadbeef);
    ret = CryptVerifyDetachedMessageHash(&para, detachedHashContent,
     sizeof(detachedHashContent), 1, &pMsgData, &size, hash, &hashSize);
    ok(!ret && GetLastError() == ERROR_MORE_DATA,
     "expected ERROR_MORE_DATA, got %08x\n", GetLastError());
    hashSize = sizeof(hash);
    ret = CryptVerifyDetachedMessageHash(&para, detachedHashContent,
     sizeof(detachedHashContent), 1, &pMsgData, &size, hash, &hashSize);
    ok(ret, "CryptVerifyDetachedMessageHash failed: %08x\n", GetLastError());
}

static const BYTE signedContent[] = {
0x30,0x81,0xb2,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0xa4,0x30,0x81,0xa1,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,0x02,0x03,0x04,
0x31,0x77,0x30,0x75,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x40,0x81,0xa6,0x70,
0xb3,0xef,0x59,0xd1,0x66,0xd1,0x9b,0xc0,0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,
0x0d,0x59,0xa9,0xaa,0x6e,0xe9,0x2c,0xa0,0x1e,0xee,0xc2,0x60,0xbc,0x59,0xbe,
0x3f,0x63,0x06,0x8d,0xc9,0x11,0x1d,0x23,0x64,0x92,0xef,0x2e,0xfc,0x57,0x29,
0xa4,0xaf,0xe0,0xee,0x93,0x19,0x39,0x51,0xe4,0x44,0xb8,0x0b,0x28,0xf4,0xa8,
0x0d };
static const BYTE signedWithCertEmptyContent[] = {
0x30,0x81,0xdf,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0xd1,0x30,0x81,0xce,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0xa0,0x7c,
0x30,0x7a,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,0x30,0x11,
0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,
0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,0x31,0x30,
0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,
0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,
0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,
0x00,0x30,0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,0xa3,0x16,0x30,0x14,0x30,
0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,
0xff,0x02,0x01,0x01,0x31,0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,
0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,
0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,
0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,
0x00 };
static const BYTE signedWithCertContent[] = {
0x30,0x82,0x01,0x32,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,
0xa0,0x82,0x01,0x23,0x30,0x82,0x01,0x1f,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,
0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x13,0x06,
0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x01,0xa0,0x06,0x04,0x04,0x01,
0x02,0x03,0x04,0xa0,0x7c,0x30,0x7a,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,
0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,
0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,
0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,
0x30,0x31,0x30,0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,
0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,
0x20,0x4c,0x61,0x6e,0x67,0x00,0x30,0x07,0x30,0x02,0x06,0x00,0x03,0x01,0x00,
0xa3,0x16,0x30,0x14,0x30,0x12,0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,
0x08,0x30,0x06,0x01,0x01,0xff,0x02,0x01,0x01,0x31,0x77,0x30,0x75,0x02,0x01,
0x01,0x30,0x1a,0x30,0x15,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,
0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,
0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,
0x06,0x00,0x05,0x00,0x04,0x40,0x81,0xa6,0x70,0xb3,0xef,0x59,0xd1,0x66,0xd1,
0x9b,0xc0,0x9a,0xb6,0x9a,0x5e,0x6d,0x6f,0x6d,0x0d,0x59,0xa9,0xaa,0x6e,0xe9,
0x2c,0xa0,0x1e,0xee,0xc2,0x60,0xbc,0x59,0xbe,0x3f,0x63,0x06,0x8d,0xc9,0x11,
0x1d,0x23,0x64,0x92,0xef,0x2e,0xfc,0x57,0x29,0xa4,0xaf,0xe0,0xee,0x93,0x19,
0x39,0x51,0xe4,0x44,0xb8,0x0b,0x28,0xf4,0xa8,0x0d };
static const BYTE signedWithCertWithPubKeyContent[] = {
0x30,0x81,0xfc,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x02,0xa0,
0x81,0xee,0x30,0x81,0xeb,0x02,0x01,0x01,0x31,0x0e,0x30,0x0c,0x06,0x08,0x2a,
0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x02,0x06,0x00,0xa0,0x81,
0x98,0x30,0x81,0x95,0x02,0x01,0x01,0x30,0x02,0x06,0x00,0x30,0x15,0x31,0x13,
0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,
0x61,0x6e,0x67,0x00,0x30,0x22,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,0x31,0x30,
0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x18,0x0f,0x31,0x36,0x30,0x31,0x30,
0x31,0x30,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x5a,0x30,0x15,0x31,0x13,0x30,
0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,0x4c,0x61,
0x6e,0x67,0x00,0x30,0x22,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x01,0x01,0x01,0x05,0x00,0x03,0x11,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xa3,0x16,0x30,0x14,0x30,0x12,
0x06,0x03,0x55,0x1d,0x13,0x01,0x01,0xff,0x04,0x08,0x30,0x06,0x01,0x01,0xff,
0x02,0x01,0x01,0x31,0x37,0x30,0x35,0x02,0x01,0x01,0x30,0x1a,0x30,0x15,0x31,
0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x03,0x13,0x0a,0x4a,0x75,0x61,0x6e,0x20,
0x4c,0x61,0x6e,0x67,0x00,0x02,0x01,0x01,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,
0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x30,0x04,0x06,0x00,0x05,0x00,0x04,0x00 };

static void test_verify_message_signature(void)
{
    BOOL ret;
    CRYPT_VERIFY_MESSAGE_PARA para = { 0 };
    PCCERT_CONTEXT cert;
    DWORD cbDecoded;

    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(NULL, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %08x\n", GetLastError());
    para.cbSize = sizeof(para);
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %08x\n", GetLastError());
    para.cbSize = 0;
    para.dwMsgAndCertEncodingType = PKCS_7_ASN_ENCODING;
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "Expected E_INVALIDARG, got %08x\n", GetLastError());
    para.cbSize = sizeof(para);
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, NULL);
    ok(!ret &&
     (GetLastError() == CRYPT_E_ASN1_EOD ||
      GetLastError() == OSS_BAD_ARG), /* win9x */
     "Expected CRYPT_E_ASN1_EOD, got %08x\n", GetLastError());
    /* Check whether cert is set on error */
    cert = (PCCERT_CONTEXT)0xdeadbeef;
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, 0, &cert);
    ok(cert == NULL, "Expected NULL cert\n");
    /* Check whether cbDecoded is set on error */
    cbDecoded = 0xdeadbeef;
    ret = CryptVerifyMessageSignature(&para, 0, NULL, 0, NULL, &cbDecoded,
     NULL);
    ok(!cbDecoded, "Expected 0\n");
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, dataEmptyBareContent,
     sizeof(dataEmptyBareContent), NULL, 0, NULL);
    ok(GetLastError() == CRYPT_E_ASN1_BADTAG ||
     GetLastError() == OSS_PDU_MISMATCH, /* win9x */
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, dataEmptyContent,
     sizeof(dataEmptyContent), NULL, 0, NULL);
    ok(!ret && GetLastError() == CRYPT_E_UNEXPECTED_MSG_TYPE,
     "Expected CRYPT_E_UNEXPECTED_MSG_TYPE, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, signedEmptyBareContent,
     sizeof(signedEmptyBareContent), NULL, 0, NULL);
    ok(!ret &&
     (GetLastError() == CRYPT_E_ASN1_BADTAG ||
      GetLastError() == OSS_DATA_ERROR), /* win9x */
     "Expected CRYPT_E_ASN1_BADTAG, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, signedEmptyContent,
     sizeof(signedEmptyContent), NULL, 0, NULL);
    ok(!ret &&
     (GetLastError() == CRYPT_E_NOT_FOUND ||
      GetLastError() == OSS_DATA_ERROR), /* win9x */
     "Expected CRYPT_E_NOT_FOUND, got %08x\n", GetLastError());
    SetLastError(0xdeadbeef);
    ret = CryptVerifyMessageSignature(&para, 0, signedContent,
     sizeof(signedContent), NULL, 0, NULL);
    ok(!ret &&
     (GetLastError() == CRYPT_E_NOT_FOUND ||
      GetLastError() == OSS_DATA_ERROR), /* win9x */
     "Expected CRYPT_E_NOT_FOUND, got %08x\n", GetLastError());
    /* FIXME: Windows fails with CRYPT_E_NOT_FOUND for these messages, but
     * their signer certs have invalid public keys that fail to decode.  In
     * Wine therefore the failure is an ASN error.  Need some messages with
     * valid public keys and invalid signatures to check against.
     */
    ret = CryptVerifyMessageSignature(&para, 0, signedWithCertEmptyContent,
     sizeof(signedWithCertEmptyContent), NULL, 0, NULL);
    ok(!ret, "Expected failure\n");
    ret = CryptVerifyMessageSignature(&para, 0, signedWithCertContent,
     sizeof(signedWithCertContent), NULL, 0, NULL);
    ok(!ret, "Expected failure\n");
    ret = CryptVerifyMessageSignature(&para, 0, signedWithCertWithPubKeyContent,
     sizeof(signedWithCertWithPubKeyContent), NULL, 0, NULL);
    ok(!ret, "Expected failure\n");
}

static const BYTE detachedHashBlob[] = {
0x30,0x3f,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x05,0xa0,0x32,
0x30,0x30,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x0b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0x04,0x10,0x2d,0x1b,0xbc,0x1f,0xc7,0xab,0x36,0x8d,0xdb,0x95,0xe6,
0x24,0xb9,0x66,0x7c,0x21 };
static const BYTE hashBlob[] = {
0x30,0x47,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x07,0x05,0xa0,0x3a,
0x30,0x38,0x02,0x01,0x00,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
0x02,0x05,0x05,0x00,0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
0x07,0x01,0xa0,0x06,0x04,0x04,0xde,0xad,0xbe,0xef,0x04,0x10,0x2f,0x24,0x92,
0x30,0xa8,0xe7,0xc2,0xbf,0x60,0x05,0xcc,0xd2,0x67,0x92,0x59,0xec };
static const BYTE hashVal[] = {
0x2d,0x1b,0xbc,0x1f,0xc7,0xab,0x36,0x8d,0xdb,0x95,0xe6,0x24,0xb9,0x66,0x7c,
0x21 };

static void test_hash_message(void)
{
    BOOL ret;
    CRYPT_HASH_MESSAGE_PARA para;
    static const BYTE blob1[] = { 0xde, 0xad, 0xbe, 0xef };
    static const BYTE blob2[] = { 0xba, 0xad, 0xf0, 0x0d };
    const BYTE *toHash[] = { blob1, blob2 };
    DWORD hashSize[] = { sizeof(blob1), sizeof(blob2) };
    DWORD hashedBlobSize, computedHashSize;
    static char oid_rsa_md5[] = szOID_RSA_MD5;
    LPBYTE hashedBlob, computedHash;

    /* Crash
    ret = CryptHashMessage(NULL, FALSE, 0, NULL, 0, NULL, NULL, NULL, NULL);
     */
    memset(&para, 0, sizeof(para));
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 0, NULL, NULL, NULL, NULL, NULL, NULL);
    ok(!ret && GetLastError() == E_INVALIDARG,
     "expected E_INVALIDARG, got 0x%08x\n", GetLastError());
    para.cbSize = sizeof(para);
    /* Not quite sure what "success" means in this case, but it does succeed */
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 0, NULL, NULL, NULL, NULL, NULL, NULL);
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    /* With a bogus encoding type it "succeeds" */
    para.dwMsgEncodingType = 0xdeadbeef;
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 0, NULL, NULL, NULL, NULL, NULL, NULL);
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    /* According to MSDN, the third parameter (cToBeHashed) must be 1 if the
     * second parameter (fDetached) is FALSE, but again it "succeeds."
     */
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 2, NULL, NULL, NULL, NULL, NULL, NULL);
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    /* Even passing parameters to hash results in "success." */
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 2, toHash, hashSize, NULL, NULL, NULL,
     NULL);
    /* Try again with a valid encoding type */
    para.dwMsgEncodingType = PKCS_7_ASN_ENCODING;
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 2, NULL, NULL, NULL, NULL, NULL, NULL);
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    /* And with valid data to hash */
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 2, toHash, hashSize, NULL, NULL, NULL,
     NULL);
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    /* But requesting the size of the hashed blob and indicating there's data
     * to hash results in a crash
     */
    if (0)
    {
        ret = CryptHashMessage(&para, FALSE, 2, NULL, NULL, NULL,
         &hashedBlobSize, NULL, NULL);
    }
    /* Passing a valid pointer for the data to hash fails, as the hash
     * algorithm is finally checked.
     */
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 2, toHash, hashSize, NULL,
     &hashedBlobSize, NULL, NULL);
    ok(!ret &&
     (GetLastError() == CRYPT_E_UNKNOWN_ALGO ||
      GetLastError() == CRYPT_E_OID_FORMAT), /* Vista */
     "expected CRYPT_E_UNKNOWN_ALGO or CRYPT_E_OID_FORMAT, got 0x%08x (%d)\n",
     GetLastError(), GetLastError());
    para.HashAlgorithm.pszObjId = oid_rsa_md5;
    /* With a valid hash algorithm, this succeeds, even though fDetached is
     * FALSE.
     */
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 2, toHash, hashSize, NULL,
     &hashedBlobSize, NULL, NULL);
    todo_wine
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    if (ret)
    {
        /* Actually attempting to get the hashed data fails, perhaps because
         * detached is FALSE.
         */
        hashedBlob = HeapAlloc(GetProcessHeap(), 0, hashedBlobSize);
        SetLastError(0xdeadbeef);
        ret = CryptHashMessage(&para, FALSE, 2, toHash, hashSize, hashedBlob,
         &hashedBlobSize, NULL, NULL);
        ok(!ret && GetLastError() == CRYPT_E_MSG_ERROR,
         "expected CRYPT_E_MSG_ERROR, got 0x%08x (%d)\n", GetLastError(),
         GetLastError());
        HeapFree(GetProcessHeap(), 0, hashedBlob);
    }
    /* Repeating tests with fDetached = TRUE results in success */
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, TRUE, 2, toHash, hashSize, NULL,
     &hashedBlobSize, NULL, NULL);
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    if (ret)
    {
        hashedBlob = HeapAlloc(GetProcessHeap(), 0, hashedBlobSize);
        SetLastError(0xdeadbeef);
        ret = CryptHashMessage(&para, TRUE, 2, toHash, hashSize, hashedBlob,
         &hashedBlobSize, NULL, NULL);
        ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
        ok(hashedBlobSize == sizeof(detachedHashBlob),
         "unexpected size of detached blob %d\n", hashedBlobSize);
        ok(!memcmp(hashedBlob, detachedHashBlob, hashedBlobSize),
         "unexpected detached blob value\n");
        HeapFree(GetProcessHeap(), 0, hashedBlob);
    }
    /* Hashing a single item with fDetached = FALSE also succeeds */
    SetLastError(0xdeadbeef);
    ret = CryptHashMessage(&para, FALSE, 1, toHash, hashSize, NULL,
     &hashedBlobSize, NULL, NULL);
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    if (ret)
    {
        hashedBlob = HeapAlloc(GetProcessHeap(), 0, hashedBlobSize);
        ret = CryptHashMessage(&para, FALSE, 1, toHash, hashSize, hashedBlob,
         &hashedBlobSize, NULL, NULL);
        ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
        ok(hashedBlobSize == sizeof(hashBlob),
         "unexpected size of detached blob %d\n", hashedBlobSize);
        ok(!memcmp(hashedBlob, hashBlob, hashedBlobSize),
         "unexpected detached blob value\n");
        HeapFree(GetProcessHeap(), 0, hashedBlob);
    }
    /* Check the computed hash value too.  You don't need to get the encoded
     * blob to get it.
     */
    computedHashSize = 0xdeadbeef;
    ret = CryptHashMessage(&para, TRUE, 2, toHash, hashSize, NULL,
     &hashedBlobSize, NULL, &computedHashSize);
    ok(ret, "CryptHashMessage failed: 0x%08x\n", GetLastError());
    ok(computedHashSize == 16, "expected hash size of 16, got %d\n",
     computedHashSize);
    if (ret)
    {
        computedHash = HeapAlloc(GetProcessHeap(), 0, computedHashSize);
        SetLastError(0xdeadbeef);
        ret = CryptHashMessage(&para, TRUE, 2, toHash, hashSize, NULL,
         &hashedBlobSize, computedHash, &computedHashSize);
        ok(computedHashSize == sizeof(hashVal),
         "unexpected size of hash value %d\n", computedHashSize);
        ok(!memcmp(computedHash, hashVal, computedHashSize),
         "unexpected value\n");
        HeapFree(GetProcessHeap(), 0, computedHash);
    }
}

START_TEST(message)
{
    test_msg_get_signer_count();
    test_verify_detached_message_hash();
    test_verify_message_signature();
    test_hash_message();
}
