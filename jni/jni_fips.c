/* jni_fips.c
 *
 * Copyright (C) 2006-2016 wolfSSL Inc.
 *
 * This file is part of wolfSSL. (formerly known as CyaSSL)
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __ANDROID__
    #include <wolfssl/options.h>
#endif

#include <com_wolfssl_wolfcrypt_WolfCrypt.h>
#include <com_wolfssl_wolfcrypt_Fips.h>
#include <wolfcrypt_jni_NativeStruct.h>
#include <wolfcrypt_jni_error.h>

#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/fips_test.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/des3.h>
#include <wolfssl/wolfcrypt/sha.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/sha512.h>
#include <wolfssl/wolfcrypt/hmac.h>
#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <cyassl/ctaocrypt/dh.h>
#include <cyassl/ctaocrypt/ecc.h>

/* #define WOLFCRYPT_JNI_DEBUG_ON */
#include <wolfcrypt_jni_debug.h>

extern JavaVM* g_vm;
static jobject g_errCb;

void NativeErrorCallback(const int ok, const int err, const char * const hash)
{
    JNIEnv* env;
    jclass class;
    jmethodID method;
    jint ret;

    ret = (int) ((*g_vm)->GetEnv(g_vm, (void**) &env, JNI_VERSION_1_6));
    if (ret == JNI_EDETACHED) {
#ifdef __ANDROID__
        ret = (*g_vm)->AttachCurrentThread(g_vm, &env, NULL);
#else
        ret = (*g_vm)->AttachCurrentThread(g_vm, (void**) &env, NULL);
#endif
        if (ret) {
            printf("Failed to attach JNIEnv to thread\n");
            return;
        }
    }
    else if (ret != JNI_OK) {
        printf("Unable to get JNIEnv from JavaVM\n");
        return;
    }

    if (JNIGlobalRefType != (*env)->GetObjectRefType(env, g_errCb))
        throwWolfCryptException(env, "Invalid errorCallback reference");
    else if (!(class = (*env)->GetObjectClass(env, g_errCb)))
        throwWolfCryptException(env, "Failed to get callback class");
    else if (!(method = (*env)->GetMethodID(env, class, "errorCallback",
        "(IILjava/lang/String;)V")))
        throwWolfCryptException(env, "Failed to get method ID");
    else
        (*env)->CallVoidMethod(env, g_errCb, method, ok, err,
            (*env)->NewStringUTF(env, hash));
}

JNIEXPORT void JNICALL Java_com_wolfssl_wolfcrypt_Fips_wolfCrypt_1SetCb_1fips(
    JNIEnv* env, jclass class, jobject callback)
{
    if ((g_errCb = (*env)->NewGlobalRef(env, callback)))
        wolfCrypt_SetCb_fips(NativeErrorCallback);
    else
        throwWolfCryptException(env, "Failed to store global error callback");
}

JNIEXPORT jstring JNICALL Java_com_wolfssl_wolfcrypt_Fips_wolfCrypt_1GetCoreHash_1fips(
    JNIEnv* env, jclass class)
{
    return (*env)->NewStringUTF(env, wolfCrypt_GetCoreHash_fips());
}

/*
 * ### FIPS Aprooved Security Methods ##########################################
 */

/*
 * wolfCrypt FIPS API - Symmetric encrypt/decrypt Service
 */

/* AES */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesSetKey_1fips__Lcom_wolfssl_wolfcrypt_Aes_2Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2I(
    JNIEnv* env, jclass class, jobject aes_object, jobject key_buffer,
    jlong size, jobject iv_buffer, jint dir)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_AES)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* key = getDirectBufferAddress(env, key_buffer);
    byte* iv = getDirectBufferAddress(env, iv_buffer);

    if (!aes || !key)
        return BAD_FUNC_ARG;

    ret = AesSetKey_fips(aes, key, size, iv, dir);

    LogStr("AesSetKey_fips(aes=%p, key, iv, %s) = %d\n", aes,
        dir ? "dec" : "enc", ret);
    LogStr("key[%u]: [%p]\n", (word32)size, key);
    LogHex(key, size);
    LogStr("iv[%u]: [%p]\n", (word32)AES_BLOCK_SIZE, iv);
    LogHex(iv, AES_BLOCK_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesSetKey_1fips__Lcom_wolfssl_wolfcrypt_Aes_2_3BJ_3BI(
    JNIEnv* env, jclass class, jobject aes_object, jbyteArray key_buffer,
    jlong size, jbyteArray iv_buffer, jint dir)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_AES)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* key = getByteArray(env, key_buffer);
    byte* iv = getByteArray(env, iv_buffer);

    ret = (!aes || !key) ? BAD_FUNC_ARG
                         : AesSetKey_fips(aes, key, size, iv, dir);

    LogStr("AesSetKey_fips(aes=%p, key, iv, %s) = %d\n", aes,
        dir ? "dec" : "enc", ret);
    LogStr("key[%u]: [%p]\n", (word32)size, key);
    LogHex(key, size);
    LogStr("iv[%u]: [%p]\n", (word32)AES_BLOCK_SIZE, iv);
    LogHex(iv, AES_BLOCK_SIZE);

    releaseByteArray(env, key_buffer, key, 1);
    releaseByteArray(env,  iv_buffer,  iv, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesSetIV_1fips__Lcom_wolfssl_wolfcrypt_Aes_2Ljava_nio_ByteBuffer_2(
    JNIEnv* env, jclass class, jobject aes_object, jobject iv_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_AES)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* iv = getDirectBufferAddress(env, iv_buffer);

    if (!aes || !iv)
        return BAD_FUNC_ARG;

    ret = AesSetIV_fips(aes, iv);

    LogStr("AesSetIV_fips(aes=%p, iv) = %d\n", aes, ret);
    LogStr("iv[%u]: [%p]\n", (word32)AES_BLOCK_SIZE, iv);
    LogHex(iv, AES_BLOCK_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesSetIV_1fips__Lcom_wolfssl_wolfcrypt_Aes_2_3B(
    JNIEnv* env, jclass class, jobject aes_object, jbyteArray iv_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_AES)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* iv = getByteArray(env, iv_buffer);

    ret = (!aes || !iv) ? BAD_FUNC_ARG
                        : AesSetIV_fips(aes, iv);

    LogStr("AesSetIV_fips(aes=%p, iv) = %d\n", aes, ret);
    LogStr("iv[%u]: [%p]\n", (word32)AES_BLOCK_SIZE, iv);
    LogHex(iv, AES_BLOCK_SIZE);

    releaseByteArray(env, iv_buffer, iv, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesCbcEncrypt_1fips__Lcom_wolfssl_wolfcrypt_Aes_2Ljava_nio_ByteBuffer_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject aes_object, jobject out_buffer,
    jobject in_buffer, jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_AES)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* out = getDirectBufferAddress(env, out_buffer);
    byte* in = getDirectBufferAddress(env, in_buffer);

    if (!aes || !out || !in)
        return BAD_FUNC_ARG;

    ret = AesCbcEncrypt_fips(aes, out, in, (word32) size);

    LogStr("AesCbcEncrypt_fips(aes=%p, out, in) = %d\n", aes, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesCbcEncrypt_1fips__Lcom_wolfssl_wolfcrypt_Aes_2_3B_3BJ(
    JNIEnv* env, jclass class, jobject aes_object, jbyteArray out_buffer,
    jbyteArray in_buffer, jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_AES)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* out = getByteArray(env, out_buffer);
    byte* in = getByteArray(env, in_buffer);

    ret = (!aes || !out || !in)
        ? BAD_FUNC_ARG
        : AesCbcEncrypt_fips(aes, out, in, (word32) size);

    LogStr("AesCbcEncrypt_fips(aes=%p, out, in) = %d\n", aes, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);

    releaseByteArray(env, out_buffer, out, ret);
    releaseByteArray(env,  in_buffer,  in, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesCbcDecrypt_1fips__Lcom_wolfssl_wolfcrypt_Aes_2Ljava_nio_ByteBuffer_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject aes_object, jobject out_buffer,
    jobject in_buffer, jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_AES)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* out = getDirectBufferAddress(env, out_buffer);
    byte* in = getDirectBufferAddress(env, in_buffer);

    if (!aes || !out || !in)
        return BAD_FUNC_ARG;

    ret = AesCbcDecrypt_fips(aes, out, in, (word32) size);

    LogStr("AesCbcDecrypt_fips(aes=%p, out, in) = %d\n", aes, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesCbcDecrypt_1fips__Lcom_wolfssl_wolfcrypt_Aes_2_3B_3BJ(
    JNIEnv* env, jclass class, jobject aes_object, jbyteArray out_buffer,
    jbyteArray in_buffer, jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_AES)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* out = getByteArray(env, out_buffer);
    byte* in = getByteArray(env, in_buffer);

    ret = (!aes || !out || !in)
        ? BAD_FUNC_ARG
        : AesCbcDecrypt_fips(aes, out, in, (word32) size);

    LogStr("AesCbcDecrypt_fips(aes=%p, out, in) = %d\n", aes, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);

    releaseByteArray(env, out_buffer, out, ret);
    releaseByteArray(env,  in_buffer,  in, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesGcmSetKey_1fips__Lcom_wolfssl_wolfcrypt_Aes_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject aes_object, jobject key_buffer,
    jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_AESGCM)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* key = getDirectBufferAddress(env, key_buffer);

    if (!aes || !key)
        return BAD_FUNC_ARG;

    ret = AesGcmSetKey_fips(aes, key, size);

    LogStr("AesGcmSetKey_fips(aes=%p, key) = %d\n", aes, ret);
    LogStr("key[%u]: [%p]\n", (word32)size, key);
    LogHex(key, size);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesGcmSetKey_1fips__Lcom_wolfssl_wolfcrypt_Aes_2_3BJ(
    JNIEnv* env, jclass class, jobject aes_object, jbyteArray key_buffer,
    jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_AESGCM)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* key = getByteArray(env, key_buffer);

    ret = (!aes || !key) ? BAD_FUNC_ARG
                         : AesGcmSetKey_fips(aes, key, size);

    LogStr("AesGcmSetKey_fips(aes=%p, key) = %d\n", aes, ret);
    LogStr("key[%u]: [%p]\n", (word32)size, key);
    LogHex(key, size);

    releaseByteArray(env, key_buffer, key, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesGcmEncrypt_1fips__Lcom_wolfssl_wolfcrypt_Aes_2Ljava_nio_ByteBuffer_2Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject aes_object, jobject out_buffer,
    jobject in_buffer, jlong size, jobject iv_buffer, jlong ivSz,
    jobject authTag_buffer, jlong authTagSz, jobject authIn_buffer,
    jlong authInSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_AESGCM)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* out = getDirectBufferAddress(env, out_buffer);
    byte* in = getDirectBufferAddress(env, in_buffer);
    byte* iv = getDirectBufferAddress(env, iv_buffer);
    byte* authTag = getDirectBufferAddress(env, authTag_buffer);
    byte* authIn = getDirectBufferAddress(env, authIn_buffer);

    if (!aes || !out || !in || (!iv && ivSz) || (!authTag && authTagSz)
        || (!authIn && authInSz))
        return BAD_FUNC_ARG;

    ret = AesGcmEncrypt_fips(aes, out, in, (word32) size, iv, (word32) ivSz,
        authTag, (word32) authTagSz, authIn, (word32) authInSz);

    LogStr(
        "AesGcmEncrypt_fips(aes=%p, out, in, iv, authTag, authIn) = %d\n",
        aes, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);
    LogStr("iv[%u]: [%p]\n", (word32)ivSz, iv);
    LogHex(iv, ivSz);
    LogStr("authTag[%u]: [%p]\n", (word32)authTagSz, authTag);
    LogHex(authTag, authTagSz);
    LogStr("authIn[%u]: [%p]\n", (word32)authInSz, authIn);
    LogHex(authIn, authInSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesGcmEncrypt_1fips__Lcom_wolfssl_wolfcrypt_Aes_2_3B_3BJ_3BJ_3BJ_3BJ(
    JNIEnv* env, jclass class, jobject aes_object, jbyteArray out_buffer,
    jbyteArray in_buffer, jlong size, jbyteArray iv_buffer, jlong ivSz,
    jbyteArray authTag_buffer, jlong authTagSz, jbyteArray authIn_buffer,
    jlong authInSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_AESGCM)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* out = getByteArray(env, out_buffer);
    byte* in = getByteArray(env, in_buffer);
    byte* iv = getByteArray(env, iv_buffer);
    byte* authTag = getByteArray(env, authTag_buffer);
    byte* authIn = getByteArray(env, authIn_buffer);

    if (!aes || !out || !in || (!iv && ivSz) || (!authTag && authTagSz)
        || (!authIn && authInSz))
        ret = BAD_FUNC_ARG;
    else
        ret = AesGcmEncrypt_fips(aes, out, in, (word32) size, iv, (word32) ivSz,
            authTag, (word32) authTagSz, authIn, (word32) authInSz);

    LogStr(
        "AesGcmEncrypt_fips(aes=%p, out, in, iv, authTag, authIn) = %d\n",
        aes, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);
    LogStr("iv[%u]: [%p]\n", (word32)ivSz, iv);
    LogHex(iv, ivSz);
    LogStr("authTag[%u]: [%p]\n", (word32)authTagSz, authTag);
    LogHex(authTag, authTagSz);
    LogStr("authIn[%u]: [%p]\n", (word32)authInSz, authIn);
    LogHex(authIn, authInSz);

    releaseByteArray(env, out_buffer, out, ret);
    releaseByteArray(env, in_buffer, in, 1);
    releaseByteArray(env, iv_buffer, iv, 1);
    releaseByteArray(env, authTag_buffer, authTag, ret);
    releaseByteArray(env, authIn_buffer, authIn, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesGcmDecrypt_1fips__Lcom_wolfssl_wolfcrypt_Aes_2Ljava_nio_ByteBuffer_2Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject aes_object, jobject out_buffer,
    jobject in_buffer, jlong size, jobject iv_buffer, jlong ivSz,
    jobject authTag_buffer, jlong authTagSz, jobject authIn_buffer,
    jlong authInSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_AESGCM)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* out = getDirectBufferAddress(env, out_buffer);
    byte* in = getDirectBufferAddress(env, in_buffer);
    byte* iv = getDirectBufferAddress(env, iv_buffer);
    byte* authTag = getDirectBufferAddress(env, authTag_buffer);
    byte* authIn = getDirectBufferAddress(env, authIn_buffer);

    if (!aes || !out || !in || (!iv && ivSz) || (!authTag && authTagSz)
        || (!authIn && authInSz))
        return BAD_FUNC_ARG;

    ret = AesGcmDecrypt_fips(aes, out, in, (word32) size, iv, (word32) ivSz,
        authTag, (word32) authTagSz, authIn, (word32) authInSz);

    LogStr(
        "AesGcmDecrypt_fips(aes=%p, out, in, iv, authTag, authIn) = %d\n",
        aes, ret);
    LogStr("in[%u]: [%p]\n", (word32)AES_BLOCK_SIZE, in);
    LogHex(in, AES_BLOCK_SIZE);
    LogStr("out[%u]: [%p]\n", (word32)AES_BLOCK_SIZE, out);
    LogHex(out, AES_BLOCK_SIZE);
    LogStr("iv[%u]: [%p]\n", (word32)ivSz, iv);
    LogHex(iv, ivSz);
    LogStr("authTag[%u]: [%p]\n", (word32)authTagSz, authTag);
    LogHex(authTag, authTagSz);
    LogStr("authIn[%u]: [%p]\n", (word32)authInSz, authIn);
    LogHex(authIn, authInSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_AesGcmDecrypt_1fips__Lcom_wolfssl_wolfcrypt_Aes_2_3B_3BJ_3BJ_3BJ_3BJ(
    JNIEnv* env, jclass class, jobject aes_object, jbyteArray out_buffer,
    jbyteArray in_buffer, jlong size, jbyteArray iv_buffer, jlong ivSz,
    jbyteArray authTag_buffer, jlong authTagSz, jbyteArray authIn_buffer,
    jlong authInSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_AESGCM)

    Aes* aes = (Aes*) getNativeStruct(env, aes_object);
    byte* out = getByteArray(env, out_buffer);
    byte* in = getByteArray(env, in_buffer);
    byte* iv = getByteArray(env, iv_buffer);
    byte* authTag = getByteArray(env, authTag_buffer);
    byte* authIn = getByteArray(env, authIn_buffer);

    if (!aes || !out || !in || (!iv && ivSz) || (!authTag && authTagSz)
        || (!authIn && authInSz))
        ret = BAD_FUNC_ARG;
    else
        ret = AesGcmDecrypt_fips(aes, out, in, (word32) size, iv, (word32) ivSz,
            authTag, (word32) authTagSz, authIn, (word32) authInSz);

    LogStr(
        "AesGcmDecrypt_fips(aes=%p, out, in, iv, authTag, authIn) = %d\n",
        aes, ret);
    LogStr("in[%u]: [%p]\n", (word32)AES_BLOCK_SIZE, in);
    LogHex(in, AES_BLOCK_SIZE);
    LogStr("out[%u]: [%p]\n", (word32)AES_BLOCK_SIZE, out);
    LogHex(out, AES_BLOCK_SIZE);
    LogStr("iv[%u]: [%p]\n", (word32)ivSz, iv);
    LogHex(iv, ivSz);
    LogStr("authTag[%u]: [%p]\n", (word32)authTagSz, authTag);
    LogHex(authTag, authTagSz);
    LogStr("authIn[%u]: [%p]\n", (word32)authInSz, authIn);
    LogHex(authIn, authInSz);

    releaseByteArray(env, out_buffer, out, ret);
    releaseByteArray(env, in_buffer, in, 1);
    releaseByteArray(env, iv_buffer, iv, 1);
    releaseByteArray(env, authTag_buffer, authTag, ret);
    releaseByteArray(env, authIn_buffer, authIn, 1);

#endif

    return ret;
}

/* DES3 */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Des3_1SetKey_1fips__Lcom_wolfssl_wolfcrypt_Des3_2Ljava_nio_ByteBuffer_2Ljava_nio_ByteBuffer_2I(
    JNIEnv* env, jclass class, jobject des_object, jobject key_buffer,
    jobject iv_buffer, jint dir)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DES3)

    Des3* des = (Des3*) getNativeStruct(env, des_object);
    byte* key = getDirectBufferAddress(env, key_buffer);
    byte* iv = getDirectBufferAddress(env, iv_buffer);

    if (!des || !key)
        return BAD_FUNC_ARG;

    ret = Des3_SetKey_fips(des, key, iv, dir);

    LogStr("Des3_SetKey_fips(des=%p, key, iv, %s) = %d\n", des,
        dir ? "dec" : "enc", ret);
    LogStr("key[%u]: [%p]\n", (word32)DES3_KEYLEN, key);
    LogHex(key, DES3_KEYLEN);
    LogStr("iv[%u]: [%p]\n", (word32)DES3_IVLEN, iv);
    LogHex(iv, DES3_IVLEN);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Des3_1SetKey_1fips__Lcom_wolfssl_wolfcrypt_Des3_2_3B_3BI(
    JNIEnv* env, jclass class, jobject des_object, jbyteArray key_buffer,
    jbyteArray iv_buffer, jint dir)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DES3)

    Des3* des = (Des3*) getNativeStruct(env, des_object);
    byte* key = getByteArray(env, key_buffer);
    byte* iv = getByteArray(env, iv_buffer);

    ret = (!des || !key) ? BAD_FUNC_ARG
                         : Des3_SetKey_fips(des, key, iv, dir);

    LogStr("Des3_SetKey_fips(des=%p, key, iv, %s) = %d\n", des,
        dir ? "dec" : "enc", ret);
    LogStr("key[%u]: [%p]\n", (word32)DES3_KEYLEN, key);
    LogHex(key, DES3_KEYLEN);
    LogStr("iv[%u]: [%p]\n", (word32)DES3_IVLEN, iv);
    LogHex(iv, DES3_IVLEN);

    releaseByteArray(env, key_buffer, key, 1);
    releaseByteArray(env, iv_buffer, iv, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Des3_1SetIV_1fips__Lcom_wolfssl_wolfcrypt_Des3_2Ljava_nio_ByteBuffer_2(
    JNIEnv* env, jclass class, jobject des_object, jobject iv_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DES3)

    Des3* des = (Des3*) getNativeStruct(env, des_object);
    byte* iv = getDirectBufferAddress(env, iv_buffer);

    if (!des || !iv)
        return BAD_FUNC_ARG;
    ret = Des3_SetIV_fips(des, iv);

    LogStr("Des3_SetIV_fips(des=%p, iv) = %d\n", des, ret);
    LogStr("iv[%u]: [%p]\n", (word32)DES_BLOCK_SIZE, iv);
    LogHex(iv, DES_BLOCK_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Des3_1SetIV_1fips__Lcom_wolfssl_wolfcrypt_Des3_2_3B(
    JNIEnv* env, jclass class, jobject des_object, jbyteArray iv_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DES3)

    Des3* des = (Des3*) getNativeStruct(env, des_object);
    byte* iv = getByteArray(env, iv_buffer);

    ret = (!des || !iv) ? BAD_FUNC_ARG
                        : Des3_SetIV_fips(des, iv);

    LogStr("Des3_SetIV_fips(des=%p, iv) = %d\n", des, ret);
    LogStr("iv[%u]: [%p]\n", (word32)DES_BLOCK_SIZE, iv);
    LogHex(iv, DES_BLOCK_SIZE);

    releaseByteArray(env, iv_buffer, iv, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Des3_1CbcEncrypt_1fips__Lcom_wolfssl_wolfcrypt_Des3_2Ljava_nio_ByteBuffer_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject des_object, jobject out_buffer,
    jobject in_buffer, jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DES3)

    Des3* des = (Des3*) getNativeStruct(env, des_object);
    byte* out = getDirectBufferAddress(env, out_buffer);
    byte* in = getDirectBufferAddress(env, in_buffer);

    if (!des || !out || !in)
        return BAD_FUNC_ARG;

    ret = Des3_CbcEncrypt_fips(des, out, in, (word32) size);

    LogStr("Des3_CbcEncrypt_fips(des=%p, out, in) = %d\n", des, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Des3_1CbcEncrypt_1fips__Lcom_wolfssl_wolfcrypt_Des3_2_3B_3BJ(
    JNIEnv* env, jclass class, jobject des_object, jbyteArray out_buffer,
    jbyteArray in_buffer, jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DES3)

    Des3* des = (Des3*) getNativeStruct(env, des_object);
    byte* out = getByteArray(env, out_buffer);
    byte* in = getByteArray(env, in_buffer);

    ret = (!des || !out || !in) ? BAD_FUNC_ARG
                                : Des3_CbcEncrypt_fips(des, out, in, (word32) size);

    LogStr("Des3_CbcEncrypt_fips(des=%p, out, in) = %d\n", des, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);

    releaseByteArray(env, out_buffer, out, ret);
    releaseByteArray(env, in_buffer, in, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Des3_1CbcDecrypt_1fips__Lcom_wolfssl_wolfcrypt_Des3_2Ljava_nio_ByteBuffer_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject des_object, jobject out_buffer,
    jobject in_buffer, jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DES3)

    Des3* des = (Des3*) getNativeStruct(env, des_object);
    byte* out = getDirectBufferAddress(env, out_buffer);
    byte* in = getDirectBufferAddress(env, in_buffer);

    if (!des || !out || !in)
        return BAD_FUNC_ARG;

    ret = Des3_CbcDecrypt_fips(des, out, in, (word32) size);

    LogStr("Des3_CbcDecrypt_fips(des=%p, out, in) = %d\n", des, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Des3_1CbcDecrypt_1fips__Lcom_wolfssl_wolfcrypt_Des3_2_3B_3BJ(
    JNIEnv* env, jclass class, jobject des_object, jbyteArray out_buffer,
    jbyteArray in_buffer, jlong size)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DES3)

    Des3* des = (Des3*) getNativeStruct(env, des_object);
    byte* out = getByteArray(env, out_buffer);
    byte* in = getByteArray(env, in_buffer);

    ret = (!des || !out || !in) ? BAD_FUNC_ARG
                                : Des3_CbcDecrypt_fips(des, out, in, (word32) size);

    LogStr("Des3_CbcDecrypt_fips(des=%p, out, in) = %d\n", des, ret);
    LogStr("in[%u]: [%p]\n", (word32)size, in);
    LogHex(in, size);
    LogStr("out[%u]: [%p]\n", (word32)size, out);
    LogHex(out, size);

    releaseByteArray(env, out_buffer, out, ret);
    releaseByteArray(env, in_buffer, in, 1);

#endif

    return ret;
}

/*
 * wolfCrypt FIPS API - Keyed hash Service
 */

/* HMAC */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_HmacSetKey_1fips__Lcom_wolfssl_wolfcrypt_Hmac_2ILjava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject hmac_object, jint type,
    jobject key_buffer, jlong keySz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_HMAC)

    Hmac* hmac = (Hmac*) getNativeStruct(env, hmac_object);
    byte* key = getDirectBufferAddress(env, key_buffer);

    if (!hmac || !key)
        return BAD_FUNC_ARG;

    ret = HmacSetKey_fips(hmac, type, key, keySz);

    LogStr("HmacSetKey_fips(hmac=%p, type=%d, key, keySz) = %d\n", hmac, type,
        ret);
    LogStr("key[%u]: [%p]\n", (word32)keySz, key);
    LogHex(key, keySz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_HmacSetKey_1fips__Lcom_wolfssl_wolfcrypt_Hmac_2I_3BJ(
    JNIEnv* env, jclass class, jobject hmac_object, jint type,
    jbyteArray key_buffer, jlong keySz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_HMAC)

    Hmac* hmac = (Hmac*) getNativeStruct(env, hmac_object);
    byte* key = getByteArray(env, key_buffer);

    ret = (!hmac || !key) ? BAD_FUNC_ARG
                          : HmacSetKey_fips(hmac, type, key, keySz);

    LogStr("HmacSetKey_fips(hmac=%p, type=%d, key, keySz) = %d\n", hmac, type,
        ret);
    LogStr("key[%u]: [%p]\n", (word32)keySz, key);
    LogHex(key, keySz);

    releaseByteArray(env, key_buffer, key, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_HmacUpdate_1fips__Lcom_wolfssl_wolfcrypt_Hmac_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject hmac_object, jobject data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_HMAC)

    Hmac* hmac = (Hmac*) getNativeStruct(env, hmac_object);
    byte* data = getDirectBufferAddress(env, data_buffer);

    if (!hmac || !data)
        return BAD_FUNC_ARG;

    ret = HmacUpdate_fips(hmac, data, len);

    LogStr("HmacUpdate_fips(hmac=%p, data, len) = %d\n", hmac, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_HmacUpdate_1fips__Lcom_wolfssl_wolfcrypt_Hmac_2_3BJ(
    JNIEnv* env, jclass class, jobject hmac_object, jbyteArray data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_HMAC)

    Hmac* hmac = (Hmac*) getNativeStruct(env, hmac_object);
    byte* data = getByteArray(env, data_buffer);

    ret = (!hmac || !data) ? BAD_FUNC_ARG
                           : HmacUpdate_fips(hmac, data, len);

    LogStr("HmacUpdate_fips(hmac=%p, data, len) = %d\n", hmac, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

    releaseByteArray(env, data_buffer, data, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_HmacFinal_1fips__Lcom_wolfssl_wolfcrypt_Hmac_2Ljava_nio_ByteBuffer_2(
    JNIEnv* env, jclass class, jobject hmac_object, jobject hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_HMAC)

    Hmac* hmac = (Hmac*) getNativeStruct(env, hmac_object);
    byte* hash = getDirectBufferAddress(env, hash_buffer);

    if (!hmac || !hash)
        return BAD_FUNC_ARG;

    ret = HmacFinal_fips(hmac, hash);

    LogStr("HmacFinal_fips(hmac=%p, hash) = %d\n", hmac, ret);
    LogStr("hash[%u]: [%p]\n", (word32)MD5_DIGEST_SIZE, hash);
    LogHex(hash, MD5_DIGEST_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_HmacFinal_1fips__Lcom_wolfssl_wolfcrypt_Hmac_2_3B(
    JNIEnv* env, jclass class, jobject hmac_object, jbyteArray hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_HMAC)

    Hmac* hmac = (Hmac*) getNativeStruct(env, hmac_object);
    byte* hash = getByteArray(env, hash_buffer);

    ret = (!hmac || !hash) ? BAD_FUNC_ARG
                           : HmacFinal_fips(hmac, hash);

    LogStr("HmacFinal_fips(hmac=%p, hash) = %d\n", hmac, ret);
    LogStr("hash[%u]: [%p]\n", (word32)MD5_DIGEST_SIZE, hash);
    LogHex(hash, MD5_DIGEST_SIZE);

    releaseByteArray(env, hash_buffer, hash, ret);

#endif

    return ret;
}

/*
 * wolfCrypt FIPS API - Random number generation Service
 */

/* RNG */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_InitRng_1fips(
    JNIEnv* env, jclass class, jobject rng_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS)

    RNG* rng = (RNG*) getNativeStruct(env, rng_object);

    if (!rng)
        return BAD_FUNC_ARG;

    ret = InitRng_fips(rng);

    LogStr("InitRng_fips(rng=%p) = %d\n", rng, ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_FreeRng_1fips(
    JNIEnv* env, jclass class, jobject rng_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS)

    RNG* rng = (RNG*) getNativeStruct(env, rng_object);

    if (!rng)
        return BAD_FUNC_ARG;

    ret = FreeRng_fips(rng);

    LogStr("FreeRng_fips(rng=%p) = %d\n", rng, ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RNG_1GenerateBlock_1fips__Lcom_wolfssl_wolfcrypt_Rng_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject rng_object, jobject buf_buffer,
    jlong bufSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS)

    RNG* rng = (RNG*) getNativeStruct(env, rng_object);
    byte* buf = getDirectBufferAddress(env, buf_buffer);

    if (!rng || !buf)
        return BAD_FUNC_ARG;

    ret = RNG_GenerateBlock_fips(rng, buf, bufSz);

    LogStr("RNG_GenerateBlock_fips(rng=%p, buf, bufSz) = %d\n", rng, ret);
    LogStr("output[%u]: [%p]\n", (word32)bufSz, buf);
    LogHex(buf, bufSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RNG_1GenerateBlock_1fips__Lcom_wolfssl_wolfcrypt_Rng_2_3BJ(
    JNIEnv* env, jclass class, jobject rng_object, jbyteArray buf_buffer,
    jlong bufSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS)

    RNG* rng = (RNG*) getNativeStruct(env, rng_object);
    byte* buf = getByteArray(env, buf_buffer);

    ret = (!rng || !buf) ? BAD_FUNC_ARG
                         : RNG_GenerateBlock_fips(rng, buf, bufSz);

    LogStr("RNG_GenerateBlock_fips(rng=%p, buf, bufSz) = %d\n", rng, ret);
    LogStr("output[%u]: [%p]\n", (word32)bufSz, buf);
    LogHex(buf, bufSz);

    releaseByteArray(env, buf_buffer, buf, ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RNG_1HealthTest_1fips__ILjava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jint reseed, jobject entropyA_object,
    jlong entropyASz, jobject entropyB_object, jlong entropyBSz,
    jobject output_object, jlong outputSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS)

    const byte* entropyA = getDirectBufferAddress(env, entropyA_object);
    const byte* entropyB = getDirectBufferAddress(env, entropyB_object);
    byte* output = getDirectBufferAddress(env, output_object);

    if (!entropyA || (reseed && !entropyB) || !output)
        return BAD_FUNC_ARG;

    ret = RNG_HealthTest_fips(reseed, entropyA, entropyASz, entropyB,
        entropyBSz, output, outputSz);

    LogStr("RNG_HealthTest_fips(reseed=%d, entropyA, entropyASz, "
        "entropyB, entropyBSz, output, outputSz) = %d\n", reseed, ret);
    LogStr("entropyA[%u]: [%p]\n", (word32)entropyASz, entropyA);
    LogHex((byte*) entropyA, entropyASz);
    LogStr("entropyB[%u]: [%p]\n", (word32)entropyBSz, entropyB);
    LogHex((byte*) entropyB, entropyBSz);
    LogStr("output[%u]: [%p]\n", (word32)outputSz, output);
    LogHex(output, outputSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RNG_1HealthTest_1fips__I_3BJ_3BJ_3BJ(
    JNIEnv* env, jclass class, jint reseed, jbyteArray entropyA_object,
    jlong entropyASz, jbyteArray entropyB_object, jlong entropyBSz,
    jbyteArray output_object, jlong outputSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS)

    const byte* entropyA = getByteArray(env, entropyA_object);
    const byte* entropyB = getByteArray(env, entropyB_object);
    byte* output = getByteArray(env, output_object);

    ret = (!entropyA || (reseed && !entropyB) || !output)
        ? BAD_FUNC_ARG
        : RNG_HealthTest_fips(reseed, entropyA, entropyASz, entropyB,
            entropyBSz, output, outputSz);

    LogStr("RNG_HealthTest_fips(reseed=%d, entropyA, entropyASz, "
        "entropyB, entropyBSz, output, outputSz) = %d\n", reseed, ret);
    LogStr("entropyA[%u]: [%p]\n", (word32)entropyASz, entropyA);
    LogHex((byte*) entropyA, entropyASz);
    LogStr("entropyB[%u]: [%p]\n", (word32)entropyBSz, entropyB);
    LogHex((byte*) entropyB, entropyBSz);
    LogStr("output[%u]: [%p]\n", (word32)outputSz, output);
    LogHex(output, outputSz);

    releaseByteArray(env, entropyA_object, (byte*)entropyA, 1);
    releaseByteArray(env, entropyB_object, (byte*)entropyB, 1);
    releaseByteArray(env, output_object, output, ret);

#endif

    return ret;
}

/*
 * wolfCrypt FIPS API - Digital signature Service
 */

/* RSA */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_InitRsaKey_1fips(
    JNIEnv* env, jclass class, jobject rsa_object, jobject heap_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);
    void* heap = getDirectBufferAddress(env, heap_object);

    if (!key)
        return BAD_FUNC_ARG;

    ret = InitRsaKey_fips(key, heap);

    LogStr("InitRsaKey_fips(key=%p, heap=%p) = %d\n", key, heap, ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_FreeRsaKey_1fips(
    JNIEnv* env, jclass class, jobject rsa_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    if (!key)
        return BAD_FUNC_ARG;

    ret = FreeRsaKey_fips(key);

    LogStr("FreeRsaKey_fips(key=%p) = %d\n", key, ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaSSL_1Sign_1fips__Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLcom_wolfssl_wolfcrypt_Rsa_2Lcom_wolfssl_wolfcrypt_Rng_2(
    JNIEnv* env, jclass class, jobject in_object, jlong inLen,
    jobject out_object, jlong outLen, jobject rsa_object, jobject rng_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    byte* in = getDirectBufferAddress(env, in_object);
    byte* out = getDirectBufferAddress(env, out_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);
    RNG* rng = (RNG*) getNativeStruct(env, rsa_object);

    /**
     * Providing an rng is optional. RNG_GenerateBlock will return BAD_FUNC_ARG
     * on a NULL rng if an RNG is needed by RsaPad.
     */
    if (!in || !out || !key)
        return BAD_FUNC_ARG;

    ret = RsaSSL_Sign_fips(in, inLen, out, outLen, key, rng);

    LogStr("RsaSSL_Sign_fips(in, inLen, out, outLen, key=%p, rng=%p) = %d\n",
        key, rng, ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex((byte*) in, inLen);
    LogStr("out[%u]: [%p]\n", (word32)outLen, out);
    LogHex((byte*) out, outLen);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaSSL_1Sign_1fips___3BJ_3BJLcom_wolfssl_wolfcrypt_Rsa_2Lcom_wolfssl_wolfcrypt_Rng_2(
    JNIEnv* env, jclass class, jbyteArray in_object, jlong inLen,
    jbyteArray out_object, jlong outLen, jobject rsa_object, jobject rng_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    byte* in = getByteArray(env, in_object);
    byte* out = getByteArray(env, out_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);
    RNG* rng = (RNG*) getNativeStruct(env, rsa_object);

    /**
     * Providing an rng is optional. RNG_GenerateBlock will return BAD_FUNC_ARG
     * on a NULL rng if an RNG is needed by RsaPad.
     */
    ret = (!in || !out || !key)
        ? BAD_FUNC_ARG
        : RsaSSL_Sign_fips(in, inLen, out, outLen, key, rng);

    LogStr("RsaSSL_Sign_fips(in, inLen, out, outLen, key=%p, rng=%p) = %d\n",
        key, rng, ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex((byte*) in, inLen);
    LogStr("out[%u]: [%p]\n", (word32)outLen, out);
    LogHex((byte*) out, outLen);

    releaseByteArray(env, in_object, in, 1);
    releaseByteArray(env, out_object, out, ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaSSL_1Verify_1fips__Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLcom_wolfssl_wolfcrypt_Rsa_2(
    JNIEnv* env, jclass class, jobject in_object, jlong inLen,
    jobject out_object, jlong outLen, jobject rsa_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    byte* in = getDirectBufferAddress(env, in_object);
    byte* out = getDirectBufferAddress(env, out_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    if (!in || !out || !key)
        return BAD_FUNC_ARG;

    ret = RsaSSL_Verify_fips(in, inLen, out, outLen, key);

    LogStr("RsaSSL_Verify_fips(in, inLen, out, outLen, key=%p) = %d\n", key,
        ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex((byte*) in, inLen);
    LogStr("out[%u]: [%p]\n", (word32)outLen, out);
    LogHex((byte*) out, outLen);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaSSL_1Verify_1fips___3BJ_3BJLcom_wolfssl_wolfcrypt_Rsa_2(
    JNIEnv* env, jclass class, jbyteArray in_object, jlong inLen,
    jbyteArray out_object, jlong outLen, jobject rsa_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    byte* in = getByteArray(env, in_object);
    byte* out = getByteArray(env, out_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    ret = (!in || !out || !key)
        ? BAD_FUNC_ARG
        : RsaSSL_Verify_fips(in, inLen, out, outLen, key);

    LogStr("RsaSSL_Verify_fips(in, inLen, out, outLen, key=%p) = %d\n", key,
        ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex((byte*) in, inLen);
    LogStr("out[%u]: [%p]\n", (word32)outLen, out);
    LogHex((byte*) out, outLen);

    releaseByteArray(env, in_object, in, 1);
    releaseByteArray(env, out_object, out, ret < 0);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaEncryptSize_1fips(
    JNIEnv* env, jclass class, jobject rsa_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    if (!key)
        return BAD_FUNC_ARG;

    ret = RsaEncryptSize_fips(key);

    LogStr("RsaEncryptSize_fips(key=%p) = %d\n", key, ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaPrivateKeyDecode_1fips__Ljava_nio_ByteBuffer_2_3JLcom_wolfssl_wolfcrypt_Rsa_2J(
    JNIEnv* env, jclass class, jobject input_object, jlongArray inOutIdx,
    jobject rsa_object, jlong inSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    word32 tmpIdx;
    byte* input = getDirectBufferAddress(env, input_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    if (!input || !key)
        return BAD_FUNC_ARG;

    (*env)->GetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpIdx);

    ret = RsaPrivateKeyDecode_fips(input, &tmpIdx, key, inSz);

    (*env)->SetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpIdx);

    LogStr("RsaPrivateKeyDecode_fips(input, inOutIdx, key=%p, inSz) = %d\n",
        key, ret);
    LogStr("input[%u]: [%p]\n", (word32)inSz, input);
    LogHex((byte*) input, inSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaPrivateKeyDecode_1fips___3B_3JLcom_wolfssl_wolfcrypt_Rsa_2J(
    JNIEnv* env, jclass class, jbyteArray input_object, jlongArray inOutIdx,
    jobject rsa_object, jlong inSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    word32 tmpIdx;
    byte* input = getByteArray(env, input_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);


    (*env)->GetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpIdx);
    LogStr("inOutIdx=%u\n", tmpIdx);

    ret = (!input || !key)
        ? BAD_FUNC_ARG
        : RsaPrivateKeyDecode_fips(input, &tmpIdx, key, inSz);

    (*env)->SetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpIdx);
    LogStr("inOutIdx=%u\n", tmpIdx);
    LogStr("RsaPrivateKeyDecode_fips(input, inOutIdx, key=%p, inSz) = %d\n",
        key, ret);
    LogStr("input[%u]: [%p]\n", (word32)inSz, input);
    LogHex((byte*) input, inSz);

    releaseByteArray(env, input_object, input, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaPublicKeyDecode_1fips__Ljava_nio_ByteBuffer_2_3JLcom_wolfssl_wolfcrypt_Rsa_2J(
    JNIEnv* env, jclass class, jobject input_object, jlongArray inOutIdx,
    jobject rsa_object, jlong inSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    word32 tmpIdx;
    byte* input = getDirectBufferAddress(env, input_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    if (!input || !key)
        return BAD_FUNC_ARG;

    (*env)->GetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpIdx);

    ret = RsaPublicKeyDecode_fips(input, &tmpIdx, key, inSz);

    (*env)->SetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpIdx);

    LogStr("RsaPublicKeyDecode_fips(input, inOutIdx, key=%p, inSz) = %d\n", key,
        ret);
    LogStr("input[%u]: [%p]\n", (word32)inSz, input);
    LogHex((byte*) input, inSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaPublicKeyDecode_1fips___3B_3JLcom_wolfssl_wolfcrypt_Rsa_2J(
    JNIEnv* env, jclass class, jbyteArray input_object, jlongArray inOutIdx,
    jobject rsa_object, jlong inSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    word32 tmpIdx;
    byte* input = getByteArray(env, input_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    (*env)->GetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpIdx);

    ret = (!input || !key)
        ? BAD_FUNC_ARG
        : RsaPublicKeyDecode_fips(input, &tmpIdx, key, inSz);

    (*env)->SetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpIdx);

    LogStr("RsaPublicKeyDecode_fips(input, inOutIdx, key=%p, inSz) = %d\n", key,
        ret);
    LogStr("input[%u]: [%p]\n", (word32)inSz, input);
    LogHex((byte*) input, inSz);

    releaseByteArray(env, input_object, input, 1);

#endif

    return ret;
}

/*
 * wolfCrypt FIPS API - Message digest Service
 */

/* SHA */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_InitSha_1fips(
    JNIEnv* env, jclass class, jobject sha_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA)

    Sha* sha = (Sha*) getNativeStruct(env, sha_object);

    if (!sha)
        return BAD_FUNC_ARG;

    ret = InitSha_fips(sha);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ShaUpdate_1fips__Lcom_wolfssl_wolfcrypt_Sha_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject sha_object, jobject data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA)

    Sha* sha = (Sha*) getNativeStruct(env, sha_object);
    byte* data = getDirectBufferAddress(env, data_buffer);

    if (!sha || !data)
        return BAD_FUNC_ARG;

    ret = ShaUpdate_fips(sha, data, len);

    LogStr("ShaUpdate_fips(sha=%p, data, len) = %d\n", sha, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ShaUpdate_1fips__Lcom_wolfssl_wolfcrypt_Sha_2_3BJ(
    JNIEnv* env, jclass class, jobject sha_object, jbyteArray data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA)

    Sha* sha = (Sha*) getNativeStruct(env, sha_object);
    byte* data = getByteArray(env, data_buffer);

    ret = (!sha || !data) ? BAD_FUNC_ARG
                          : ShaUpdate_fips(sha, data, len);

    LogStr("ShaUpdate_fips(sha=%p, data, len) = %d\n", sha, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

    releaseByteArray(env, data_buffer, data, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ShaFinal_1fips__Lcom_wolfssl_wolfcrypt_Sha_2Ljava_nio_ByteBuffer_2(
    JNIEnv* env, jclass class, jobject sha_object, jobject hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA)

    Sha* sha = (Sha*) getNativeStruct(env, sha_object);
    byte* hash = getDirectBufferAddress(env, hash_buffer);

    if (!sha || !hash)
        return BAD_FUNC_ARG;

    ret = ShaFinal_fips(sha, hash);

    LogStr("ShaFinal_fips(sha=%p, hash) = %d\n", sha, ret);
    LogStr("hash[%u]: [%p]\n", (word32)SHA_DIGEST_SIZE, hash);
    LogHex(hash, SHA_DIGEST_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ShaFinal_1fips__Lcom_wolfssl_wolfcrypt_Sha_2_3B(
    JNIEnv* env, jclass class, jobject sha_object, jbyteArray hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA)

    Sha* sha = (Sha*) getNativeStruct(env, sha_object);
    byte* hash = getByteArray(env, hash_buffer);

    ret = (!sha || !hash) ? BAD_FUNC_ARG
                          : ShaFinal_fips(sha, hash);

    LogStr("ShaFinal_fips(sha=%p, hash) = %d\n", sha, ret);
    LogStr("hash[%u]: [%p]\n", (word32)SHA_DIGEST_SIZE, hash);
    LogHex(hash, SHA_DIGEST_SIZE);

    releaseByteArray(env, hash_buffer, hash, ret);

#endif

    return ret;
}

/* SHA256 */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_InitSha256_1fips(
    JNIEnv* env, jclass class, jobject sha_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA256)

    Sha256* sha = (Sha256*) getNativeStruct(env, sha_object);

    if (!sha)
        return BAD_FUNC_ARG;

    ret = InitSha256_fips(sha);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha256Update_1fips__Lcom_wolfssl_wolfcrypt_Sha256_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject sha_object, jobject data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA256)

    Sha256* sha = (Sha256*) getNativeStruct(env, sha_object);
    byte* data = getDirectBufferAddress(env, data_buffer);

    if (!sha || !data)
        return BAD_FUNC_ARG;

    ret = Sha256Update_fips(sha, data, len);

    LogStr("Sha256Update_fips(sha=%p, data, len) = %d\n", sha, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha256Update_1fips__Lcom_wolfssl_wolfcrypt_Sha256_2_3BJ(
    JNIEnv* env, jclass class, jobject sha_object, jbyteArray data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA256)

    Sha256* sha = (Sha256*) getNativeStruct(env, sha_object);
    byte* data = getByteArray(env, data_buffer);

    ret = (!sha || !data) ? BAD_FUNC_ARG
                          : Sha256Update_fips(sha, data, len);

    LogStr("Sha256Update_fips(sha=%p, data, len) = %d\n", sha, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

    releaseByteArray(env, data_buffer, data, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha256Final_1fips__Lcom_wolfssl_wolfcrypt_Sha256_2Ljava_nio_ByteBuffer_2(
    JNIEnv* env, jclass class, jobject sha_object, jobject hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA256)

    Sha256* sha = (Sha256*) getNativeStruct(env, sha_object);
    byte* hash = getDirectBufferAddress(env, hash_buffer);

    if (!sha || !hash)
        return BAD_FUNC_ARG;

    ret = Sha256Final_fips(sha, hash);

    LogStr("Sha256Final_fips(sha=%p, hash) = %d\n", sha, ret);
    LogStr("hash[%u]: [%p]\n", (word32)SHA256_DIGEST_SIZE, hash);
    LogHex(hash, SHA256_DIGEST_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha256Final_1fips__Lcom_wolfssl_wolfcrypt_Sha256_2_3B(
    JNIEnv* env, jclass class, jobject sha_object, jbyteArray hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_SHA256)

    Sha256* sha = (Sha256*) getNativeStruct(env, sha_object);
    byte* hash = getByteArray(env, hash_buffer);

    ret = (!sha || !hash) ? BAD_FUNC_ARG
                          : Sha256Final_fips(sha, hash);

    LogStr("Sha256Final_fips(sha=%p, hash) = %d\n", sha, ret);
    LogStr("hash[%u]: [%p]\n", (word32)SHA256_DIGEST_SIZE, hash);
    LogHex(hash, SHA256_DIGEST_SIZE);

    releaseByteArray(env, hash_buffer, hash, ret);

#endif

    return ret;
}

/* SHA384 */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_InitSha384_1fips(
    JNIEnv* env, jclass class, jobject sha_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha384* sha = (Sha384*) getNativeStruct(env, sha_object);

    if (!sha)
        return BAD_FUNC_ARG;

    ret = InitSha384_fips(sha);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha384Update_1fips__Lcom_wolfssl_wolfcrypt_Sha384_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject sha_object, jobject data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha384* sha = (Sha384*) getNativeStruct(env, sha_object);
    byte* data = getDirectBufferAddress(env, data_buffer);

    if (!sha || !data)
        return BAD_FUNC_ARG;

    ret = Sha384Update_fips(sha, data, len);

    LogStr("Sha384Update_fips(sha=%p, data, len) = %d\n", sha, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha384Update_1fips__Lcom_wolfssl_wolfcrypt_Sha384_2_3BJ(
    JNIEnv* env, jclass class, jobject sha_object, jbyteArray data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha384* sha = (Sha384*) getNativeStruct(env, sha_object);
    byte* data = getByteArray(env, data_buffer);

    ret = (!sha || !data) ? BAD_FUNC_ARG
                          : Sha384Update_fips(sha, data, len);

    LogStr("Sha384Update_fips(sha=%p, data, len) = %d\n", sha, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

    releaseByteArray(env, data_buffer, data, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha384Final_1fips__Lcom_wolfssl_wolfcrypt_Sha384_2Ljava_nio_ByteBuffer_2(
    JNIEnv* env, jclass class, jobject sha_object, jobject hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha384* sha = (Sha384*) getNativeStruct(env, sha_object);
    byte* hash = getDirectBufferAddress(env, hash_buffer);

    if (!sha || !hash)
        return BAD_FUNC_ARG;

    ret = Sha384Final_fips(sha, hash);

    LogStr("Sha384Final_fips(sha=%p, hash) = %d\n", sha, ret);
    LogStr("hash[%u]: [%p]\n", (word32)SHA384_DIGEST_SIZE, hash);
    LogHex(hash, SHA384_DIGEST_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha384Final_1fips__Lcom_wolfssl_wolfcrypt_Sha384_2_3B(
    JNIEnv* env, jclass class, jobject sha_object, jbyteArray hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha384* sha = (Sha384*) getNativeStruct(env, sha_object);
    byte* hash = getByteArray(env, hash_buffer);

    ret = (!sha || !hash) ? BAD_FUNC_ARG
                          : Sha384Final_fips(sha, hash);

    LogStr("Sha384Final_fips(sha=%p, hash) = %d\n", sha, ret);
    LogStr("hash[%u]: [%p]\n", (word32)SHA384_DIGEST_SIZE, hash);
    LogHex(hash, SHA384_DIGEST_SIZE);

    releaseByteArray(env, hash_buffer, hash, ret);

#endif

    return ret;
}

/* SHA512 */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_InitSha512_1fips(
    JNIEnv* env, jclass class, jobject sha_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha512* sha = (Sha512*) getNativeStruct(env, sha_object);

    if (!sha)
        return BAD_FUNC_ARG;

    ret = InitSha512_fips(sha);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha512Update_1fips__Lcom_wolfssl_wolfcrypt_Sha512_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject sha_object, jobject data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha512* sha = (Sha512*) getNativeStruct(env, sha_object);
    byte* data = getDirectBufferAddress(env, data_buffer);

    if (!sha || !data)
        return BAD_FUNC_ARG;

    ret = Sha512Update_fips(sha, data, len);

    LogStr("Sha512Update_fips(sha=%p, data, len) = %d\n", sha, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha512Update_1fips__Lcom_wolfssl_wolfcrypt_Sha512_2_3BJ(
    JNIEnv* env, jclass class, jobject sha_object, jbyteArray data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha512* sha = (Sha512*) getNativeStruct(env, sha_object);
    byte* data = getByteArray(env, data_buffer);

    ret = (!sha || !data) ? BAD_FUNC_ARG
                          : Sha512Update_fips(sha, data, len);

    LogStr("Sha512Update_fips(sha=%p, data, len) = %d\n", sha, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

    releaseByteArray(env, data_buffer, data, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha512Final_1fips__Lcom_wolfssl_wolfcrypt_Sha512_2Ljava_nio_ByteBuffer_2(
    JNIEnv* env, jclass class, jobject sha_object, jobject hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha512* sha = (Sha512*) getNativeStruct(env, sha_object);
    byte* hash = getDirectBufferAddress(env, hash_buffer);

    if (!sha || !hash)
        return BAD_FUNC_ARG;

    ret = Sha512Final_fips(sha, hash);

    LogStr("Sha512Final_fips(sha=%p, hash) = %d\n", sha, ret);
    LogStr("hash[%u]: [%p]\n", (word32)SHA512_DIGEST_SIZE, hash);
    LogHex(hash, SHA512_DIGEST_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Sha512Final_1fips__Lcom_wolfssl_wolfcrypt_Sha512_2_3B(
    JNIEnv* env, jclass class, jobject sha_object, jbyteArray hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(WOLFSSL_SHA512)

    Sha512* sha = (Sha512*) getNativeStruct(env, sha_object);
    byte* hash = getByteArray(env, hash_buffer);

    ret = (!sha || !hash) ? BAD_FUNC_ARG
                          : Sha512Final_fips(sha, hash);

    LogStr("Sha512Final_fips(sha=%p, hash) = %d\n", sha, ret);
    LogStr("hash[%u]: [%p]\n", (word32)SHA512_DIGEST_SIZE, hash);
    LogHex(hash, SHA512_DIGEST_SIZE);

    releaseByteArray(env, hash_buffer, hash, ret);

#endif

    return ret;
}

/*
 * wolfCrypt FIPS API - Show status Service
 */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_wolfCrypt_1GetStatus_1fips(
    JNIEnv* env, jclass class)
{
    return (jint) wolfCrypt_GetStatus_fips();
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_wolfCrypt_1SetStatus_1fips(
    JNIEnv* env, jclass class, jint status)
{
#ifdef HAVE_FORCE_FIPS_FAILURE
    return (jint) wolfCrypt_SetStatus_fips(status);
#else
    return NOT_COMPILED_IN;
#endif
}

/*
 * ### FIPS Allowed Security Methods ###########################################
 */

/*
 * wolfCrypt FIPS API - Key transport Service
 */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaPublicEncrypt_1fips__Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLcom_wolfssl_wolfcrypt_Rsa_2Lcom_wolfssl_wolfcrypt_Rng_2(
    JNIEnv* env, jclass class, jobject in_object, jlong inLen,
    jobject out_object, jlong outLen, jobject rsa_object, jobject rng_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    byte* in = getDirectBufferAddress(env, in_object);
    byte* out = getDirectBufferAddress(env, out_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);
    RNG* rng = (RNG*) getNativeStruct(env, rng_object);

    /**
     * Providing an rng is optional. RNG_GenerateBlock will return BAD_FUNC_ARG
     * on a NULL rng if an RNG is needed by RsaPad.
     */
    if (!in || !out || !key)
        return BAD_FUNC_ARG;

    ret = RsaPublicEncrypt_fips(in, inLen, out, outLen, key, rng);

    LogStr(
        "RsaPublicEncrypt_fips(in, inLen, out, outLen, key=%p, rng=%p) = %d\n",
        key, rng, ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex((byte*) in, inLen);
    LogStr("out[%u]: [%p]\n", (word32)outLen, out);
    LogHex((byte*) out, outLen);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaPublicEncrypt_1fips___3BJ_3BJLcom_wolfssl_wolfcrypt_Rsa_2Lcom_wolfssl_wolfcrypt_Rng_2(
    JNIEnv* env, jclass class, jbyteArray in_object, jlong inLen,
    jbyteArray out_object, jlong outLen, jobject rsa_object, jobject rng_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    byte* in = getByteArray(env, in_object);
    byte* out = getByteArray(env, out_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);
    RNG* rng = (RNG*) getNativeStruct(env, rng_object);

    /**
     * Providing an rng is optional. RNG_GenerateBlock will return BAD_FUNC_ARG
     * on a NULL rng if an RNG is needed by RsaPad.
     */
    ret = (!in || !out || !key)
        ? BAD_FUNC_ARG
        : RsaPublicEncrypt_fips(in, inLen, out, outLen, key, rng);

    LogStr(
        "RsaPublicEncrypt_fips(in, inLen, out, outLen, key=%p, rng=%p) = %d\n",
        key, rng, ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex((byte*) in, inLen);
    LogStr("out[%u]: [%p]\n", (word32)outLen, out);
    LogHex((byte*) out, outLen);

    releaseByteArray(env, in_object, in, 1);
    releaseByteArray(env, out_object, out, ret < 0);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaPrivateDecrypt_1fips__Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2JLcom_wolfssl_wolfcrypt_Rsa_2(
    JNIEnv* env, jclass class, jobject in_object, jlong inLen,
    jobject out_object, jlong outLen, jobject rsa_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    byte* in = getDirectBufferAddress(env, in_object);
    byte* out = getDirectBufferAddress(env, out_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    if (!in || !out || !key)
        return BAD_FUNC_ARG;

    ret = RsaPrivateDecrypt_fips(in, inLen, out, outLen, key);

    LogStr("RsaPrivateDecrypt_fips(in, inLen, out, outLen, key=%p) = %d\n", key,
        ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex((byte*) in, inLen);
    LogStr("out[%u]: [%p]\n", (word32)outLen, out);
    LogHex((byte*) out, outLen);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_RsaPrivateDecrypt_1fips___3BJ_3BJLcom_wolfssl_wolfcrypt_Rsa_2(
    JNIEnv* env, jclass class, jbyteArray in_object, jlong inLen,
    jbyteArray out_object, jlong outLen, jobject rsa_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_RSA)

    byte* in = getByteArray(env, in_object);
    byte* out = getByteArray(env, out_object);
    RsaKey* key = (RsaKey*) getNativeStruct(env, rsa_object);

    ret = (!in || !out || !key)
        ? BAD_FUNC_ARG
        : RsaPrivateDecrypt_fips(in, inLen, out, outLen, key);

    LogStr("RsaPrivateDecrypt_fips(in, inLen, out, outLen, key=%p) = %d\n", key,
        ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex((byte*) in, inLen);
    LogStr("out[%u]: [%p]\n", (word32)outLen, out);
    LogHex((byte*) out, outLen);

    releaseByteArray(env, in_object, in, 1);
    releaseByteArray(env, out_object, out, ret < 0);

#endif

    return ret;
}

/*
 * wolfCrypt FIPS API - Message digest MD5 Service
 */

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_InitMd5_1fips(
    JNIEnv* env, jclass class, jobject md5_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_MD5)

    Md5* md5 = (Md5*) getNativeStruct(env, md5_object);

    if (!md5)
        return BAD_FUNC_ARG;

    InitMd5(md5);
    ret = com_wolfssl_wolfcrypt_WolfCrypt_SUCCESS;

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Md5Update__Lcom_wolfssl_wolfcrypt_Md5_2Ljava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject md5_object, jobject data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_MD5)

    Md5* md5 = (Md5*) getNativeStruct(env, md5_object);
    byte* data = getDirectBufferAddress(env, data_buffer);

    if (!md5 || !data)
        return BAD_FUNC_ARG;

    Md5Update(md5, data, len);
    ret = com_wolfssl_wolfcrypt_WolfCrypt_SUCCESS;

    LogStr("Md5Update_fips(md5=%p, data, len) = %d\n", md5, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Md5Update__Lcom_wolfssl_wolfcrypt_Md5_2_3BJ(
    JNIEnv* env, jclass class, jobject md5_object, jbyteArray data_buffer,
    jlong len)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_MD5)

    Md5* md5 = (Md5*) getNativeStruct(env, md5_object);
    byte* data = getByteArray(env, data_buffer);

    if (!md5 || !data)
        ret = BAD_FUNC_ARG;
    else {
        Md5Update(md5, data, len);
        ret = com_wolfssl_wolfcrypt_WolfCrypt_SUCCESS;
    }

    LogStr("Md5Update_fips(md5=%p, data, len) = %d\n", md5, ret);
    LogStr("data[%u]: [%p]\n", (word32)len, data);
    LogHex(data, len);

    releaseByteArray(env, data_buffer, data, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Md5Final__Lcom_wolfssl_wolfcrypt_Md5_2Ljava_nio_ByteBuffer_2(
    JNIEnv* env, jclass class, jobject md5_object, jobject hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_MD5)

    Md5* md5 = (Md5*) getNativeStruct(env, md5_object);
    byte* hash = getDirectBufferAddress(env, hash_buffer);

    if (!md5 || !hash)
        return BAD_FUNC_ARG;

    Md5Final(md5, hash);
    ret = com_wolfssl_wolfcrypt_WolfCrypt_SUCCESS;

    LogStr("Md5Final_fips(md5=%p, hash) = %d\n", md5, ret);
    LogStr("hash[%u]: [%p]\n", (word32)MD5_DIGEST_SIZE, hash);
    LogHex(hash, MD5_DIGEST_SIZE);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_Md5Final__Lcom_wolfssl_wolfcrypt_Md5_2_3B(
    JNIEnv* env, jclass class, jobject md5_object, jbyteArray hash_buffer)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_MD5)

    Md5* md5 = (Md5*) getNativeStruct(env, md5_object);
    byte* hash = getByteArray(env, hash_buffer);

    if (!md5 || !hash)
        ret = BAD_FUNC_ARG;
    else {
        Md5Final(md5, hash);
        ret = com_wolfssl_wolfcrypt_WolfCrypt_SUCCESS;
    }

    LogStr("Md5Final_fips(md5=%p, hash) = %d\n", md5, ret);
    LogStr("hash[%u]: [%p]\n", (word32)MD5_DIGEST_SIZE, hash);
    LogHex(hash, MD5_DIGEST_SIZE);

    releaseByteArray(env, hash_buffer, hash, ret);

#endif

    return ret;
}

/*
 * wolfCrypt FIPS API - Key agreement Service
 */

JNIEXPORT void JNICALL Java_com_wolfssl_wolfcrypt_Fips_InitDhKey(
    JNIEnv* env, jclass class, jobject key_object)
{
#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);

    if (key)
        InitDhKey(key);

#endif
}

JNIEXPORT void JNICALL Java_com_wolfssl_wolfcrypt_Fips_FreeDhKey(
    JNIEnv* env, jclass class, jobject key_object)
{
#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);

    if (key)
        FreeDhKey(key);

#endif
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhGenerateKeyPair__Lcom_wolfssl_wolfcrypt_Dh_2Lcom_wolfssl_wolfcrypt_Rng_2Ljava_nio_ByteBuffer_2_3JLjava_nio_ByteBuffer_2_3J(
    JNIEnv* env, jclass class, jobject key_object, jobject rng_object,
    jobject priv_buffer, jlongArray privSz, jobject pub_buffer,
    jlongArray pubSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);
    RNG* rng = (RNG*) getNativeStruct(env, rng_object);
    byte* priv = getDirectBufferAddress(env, priv_buffer);
    byte* pub = getDirectBufferAddress(env, pub_buffer);
    word32 tmpPrivSz, tmpPubSz;

    if (!key || !rng || !priv || !pub)
        return BAD_FUNC_ARG;

    (*env)->GetLongArrayRegion(env, privSz, 0, 1, (jlong*) &tmpPrivSz);
    (*env)->GetLongArrayRegion(env, pubSz, 0, 1, (jlong*) &tmpPubSz);

    ret = DhGenerateKeyPair(key, rng, priv, &tmpPrivSz, pub, &tmpPubSz);

    (*env)->SetLongArrayRegion(env, privSz, 0, 1, (jlong*) &tmpPrivSz);
    (*env)->SetLongArrayRegion(env, pubSz, 0, 1, (jlong*) &tmpPubSz);

    LogStr("DhGenerateKeyPair(key=%p, rng=%p, priv, privSz, pub, pubSz) = %d\n",
        key, rng, ret);
    LogStr("priv[%u]: [%p]\n", (word32)tmpPrivSz, priv);
    LogHex(priv, tmpPrivSz);
    LogStr("pub[%u]: [%p]\n", (word32)tmpPubSz, pub);
    LogHex(pub, tmpPubSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhGenerateKeyPair__Lcom_wolfssl_wolfcrypt_Dh_2Lcom_wolfssl_wolfcrypt_Rng_2_3B_3J_3B_3J(
    JNIEnv* env, jclass class, jobject key_object, jobject rng_object,
    jbyteArray priv_buffer, jlongArray privSz, jbyteArray pub_buffer,
    jlongArray pubSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);
    RNG* rng = (RNG*) getNativeStruct(env, rng_object);
    byte* priv = getByteArray(env, priv_buffer);
    byte* pub = getByteArray(env, pub_buffer);
    word32 tmpPrivSz, tmpPubSz;

    (*env)->GetLongArrayRegion(env, privSz, 0, 1, (jlong*) &tmpPrivSz);
    (*env)->GetLongArrayRegion(env, pubSz, 0, 1, (jlong*) &tmpPubSz);

    ret = (!key || !rng || !priv || !pub)
        ? BAD_FUNC_ARG
        : DhGenerateKeyPair(key, rng, priv, &tmpPrivSz, pub, &tmpPubSz);

    (*env)->SetLongArrayRegion(env, privSz, 0, 1, (jlong*) &tmpPrivSz);
    (*env)->SetLongArrayRegion(env, pubSz, 0, 1, (jlong*) &tmpPubSz);

    LogStr("DhGenerateKeyPair(key=%p, rng=%p, priv, privSz, pub, pubSz) = %d\n",
        key, rng, ret);
    LogStr("priv[%u]: [%p]\n", (word32)tmpPrivSz, priv);
    LogHex(priv, tmpPrivSz);
    LogStr("pub[%u]: [%p]\n", (word32)tmpPubSz, pub);
    LogHex(pub, tmpPubSz);

    releaseByteArray(env, priv_buffer, priv, ret < 0);
    releaseByteArray(env, pub_buffer, pub, ret < 0);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhAgree__Lcom_wolfssl_wolfcrypt_Dh_2Ljava_nio_ByteBuffer_2_3JLjava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject key_object, jobject agree_buffer,
    jlongArray agreeSz, jobject priv_buffer, jlong privSz, jobject pub_buffer,
    jlong pubSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);
    byte* agree = getDirectBufferAddress(env, agree_buffer);
    byte* priv = getDirectBufferAddress(env, priv_buffer);
    byte* pub = getDirectBufferAddress(env, pub_buffer);
    word32 tmpAgreeSz;

    if (!key || !agree || !priv || !pub)
        return BAD_FUNC_ARG;

    (*env)->GetLongArrayRegion(env, agreeSz, 0, 1, (jlong*) &tmpAgreeSz);

    ret = DhAgree(key, agree, &tmpAgreeSz, priv, privSz, pub, pubSz);

    (*env)->SetLongArrayRegion(env, agreeSz, 0, 1, (jlong*) &tmpAgreeSz);

    LogStr("DhAgree(key=%p, agree, agreeSz, priv, privSz, pub, pubSz) = %d\n",
        key, ret);
    LogStr("agree[%u]: [%p]\n", (word32)tmpAgreeSz, agree);
    LogHex(agree, tmpAgreeSz);
    LogStr("priv[%u]: [%p]\n", (word32)privSz, priv);
    LogHex(priv, privSz);
    LogStr("pub[%u]: [%p]\n", (word32)pubSz, pub);
    LogHex(pub, pubSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhAgree__Lcom_wolfssl_wolfcrypt_Dh_2_3B_3J_3BJ_3BJ(
    JNIEnv* env, jclass class, jobject key_object, jbyteArray agree_buffer,
    jlongArray agreeSz, jbyteArray priv_buffer, jlong privSz, jbyteArray pub_buffer,
    jlong pubSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);
    byte* agree = getByteArray(env, agree_buffer);
    byte* priv = getByteArray(env, priv_buffer);
    byte* pub = getByteArray(env, pub_buffer);
    word32 tmpAgreeSz;

    (*env)->GetLongArrayRegion(env, agreeSz, 0, 1, (jlong*) &tmpAgreeSz);

    ret = (!key || !agree || !priv || !pub)
        ? BAD_FUNC_ARG
        : DhAgree(key, agree, &tmpAgreeSz, priv, privSz, pub, pubSz);

    (*env)->SetLongArrayRegion(env, agreeSz, 0, 1, (jlong*) &tmpAgreeSz);

    LogStr("DhAgree(key=%p, agree, agreeSz, priv, privSz, pub, pubSz) = %d\n",
        key, ret);
    LogStr("agree[%u]: [%p]\n", (word32)tmpAgreeSz, agree);
    LogHex(agree, tmpAgreeSz);
    LogStr("priv[%u]: [%p]\n", (word32)privSz, priv);
    LogHex(priv, privSz);
    LogStr("pub[%u]: [%p]\n", (word32)pubSz, pub);
    LogHex(pub, pubSz);

    releaseByteArray(env, agree_buffer, agree, ret < 0);
    releaseByteArray(env, priv_buffer, priv, 1);
    releaseByteArray(env, pub_buffer, pub, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhKeyDecode__Ljava_nio_ByteBuffer_2_3JLcom_wolfssl_wolfcrypt_Dh_2J(
    JNIEnv* env, jclass class, jobject input_buffer, jlongArray inOutIdx,
    jobject key_object, jlong inSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);
    byte* input = getDirectBufferAddress(env, input_buffer);
    word32 tmpInOutIdx;

    if (!key || !input)
        return BAD_FUNC_ARG;

    (*env)->GetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpInOutIdx);

    ret = DhKeyDecode(input, &tmpInOutIdx, key, inSz);

    (*env)->SetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpInOutIdx);

    LogStr("DhKeyDecode(input, &inOutIdx, key=%p, inSz) = %d\n", key, ret);
    LogStr("input[%u]: [%p]\n", (word32)inSz, input);
    LogHex(input, inSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhKeyDecode___3B_3JLcom_wolfssl_wolfcrypt_Dh_2J(
    JNIEnv* env, jclass class, jbyteArray input_buffer, jlongArray inOutIdx,
    jobject key_object, jlong inSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);
    byte* input = getByteArray(env, input_buffer);
    word32 tmpInOutIdx;

    (*env)->GetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpInOutIdx);

    ret = (!key || !input)
        ? BAD_FUNC_ARG
        : DhKeyDecode(input, &tmpInOutIdx, key, inSz);

    (*env)->SetLongArrayRegion(env, inOutIdx, 0, 1, (jlong*) &tmpInOutIdx);

    LogStr("DhKeyDecode(input, &inOutIdx, key=%p, inSz) = %d\n", key, ret);
    LogStr("input[%u]: [%p]\n", (word32)inSz, input);
    LogHex(input, inSz);

    releaseByteArray(env, input_buffer, input, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhSetKey__Lcom_wolfssl_wolfcrypt_Dh_2Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2J(
    JNIEnv* env, jclass class, jobject key_object, jobject p_buffer, jlong pSz,
    jobject g_buffer, jlong gSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);
    byte* p = getDirectBufferAddress(env, p_buffer);
    byte* g = getDirectBufferAddress(env, g_buffer);

    if (!key || !p || !g)
        return BAD_FUNC_ARG;

    ret = DhSetKey(key, p, pSz, g, gSz);

    LogStr("DhSetKey(key=%p, p, pSz, g, gSz) = %d\n", key, ret);
    LogStr("p[%u]: [%p]\n", (word32)pSz, p);
    LogHex(p, pSz);
    LogStr("g[%u]: [%p]\n", (word32)gSz, g);
    LogHex(g, gSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhSetKey__Lcom_wolfssl_wolfcrypt_Dh_2_3BJ_3BJ(
    JNIEnv* env, jclass class, jobject key_object, jbyteArray p_buffer, jlong pSz,
    jbyteArray g_buffer, jlong gSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    DhKey* key = (DhKey*) getNativeStruct(env, key_object);
    byte* p = getByteArray(env, p_buffer);
    byte* g = getByteArray(env, g_buffer);

    ret = (!key || !p || !g)
        ? BAD_FUNC_ARG
        : DhSetKey(key, p, pSz, g, gSz);

    LogStr("DhSetKey(key=%p, p, pSz, g, gSz) = %d\n", key, ret);
    LogStr("p[%u]: [%p]\n", (word32)pSz, p);
    LogHex(p, pSz);
    LogStr("g[%u]: [%p]\n", (word32)gSz, g);
    LogHex(g, gSz);

    releaseByteArray(env, p_buffer, p, 1);
    releaseByteArray(env, g_buffer, g, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhParamsLoad__Ljava_nio_ByteBuffer_2JLjava_nio_ByteBuffer_2_3JLjava_nio_ByteBuffer_2_3J(
    JNIEnv* env, jclass class, jobject input_buffer, jlong inSz,
    jobject p_buffer, jlongArray pInOutSz, jobject g_buffer,
    jlongArray gInOutSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    byte* input = getDirectBufferAddress(env, input_buffer);
    byte* p = getDirectBufferAddress(env, p_buffer);
    byte* g = getDirectBufferAddress(env, g_buffer);
    word32 tmpPInOutSz, tmpGInOutSz;

    if (!input || !p || !g)
        return BAD_FUNC_ARG;

    (*env)->GetLongArrayRegion(env, pInOutSz, 0, 1, (jlong*) &tmpPInOutSz);
    (*env)->GetLongArrayRegion(env, gInOutSz, 0, 1, (jlong*) &tmpGInOutSz);

    ret = DhParamsLoad(input, inSz, p, &tmpPInOutSz, g, &tmpGInOutSz);

    (*env)->SetLongArrayRegion(env, pInOutSz, 0, 1, (jlong*) &tmpPInOutSz);
    (*env)->SetLongArrayRegion(env, gInOutSz, 0, 1, (jlong*) &tmpGInOutSz);

    LogStr("DhParamsLoad(input, inSz, p, &pInOutSz, g, &gInOutSz) = %d\n", ret);
    LogStr("input[%u]: [%p]\n", (word32)inSz, input);
    LogHex(input, inSz);
    LogStr("p[%u]: [%p]\n", (word32)tmpPInOutSz, p);
    LogHex(p, tmpPInOutSz);
    LogStr("g[%u]: [%p]\n", (word32)tmpGInOutSz, g);
    LogHex(g, tmpGInOutSz);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_DhParamsLoad___3BJ_3B_3J_3B_3J(
    JNIEnv* env, jclass class, jbyteArray input_buffer, jlong inSz,
    jbyteArray p_buffer, jlongArray pInOutSz, jbyteArray g_buffer,
    jlongArray gInOutSz)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && !defined(NO_DH)

    byte* input = getByteArray(env, input_buffer);
    byte* p = getByteArray(env, p_buffer);
    byte* g = getByteArray(env, g_buffer);
    word32 tmpPInOutSz, tmpGInOutSz;

    (*env)->GetLongArrayRegion(env, pInOutSz, 0, 1, (jlong*) &tmpPInOutSz);
    (*env)->GetLongArrayRegion(env, gInOutSz, 0, 1, (jlong*) &tmpGInOutSz);

    ret = (!input || !p || !g)
        ? BAD_FUNC_ARG
        : DhParamsLoad(input, inSz, p, &tmpPInOutSz, g, &tmpGInOutSz);

    (*env)->SetLongArrayRegion(env, pInOutSz, 0, 1, (jlong*) &tmpPInOutSz);
    (*env)->SetLongArrayRegion(env, gInOutSz, 0, 1, (jlong*) &tmpGInOutSz);

    LogStr("DhParamsLoad(input, inSz, p, &pInOutSz, g, &gInOutSz) = %d\n", ret);
    LogStr("input[%u]: [%p]\n", (word32)inSz, input);
    LogHex(input, inSz);
    LogStr("p[%u]: [%p]\n", (word32)tmpPInOutSz, p);
    LogHex(p, tmpPInOutSz);
    LogStr("g[%u]: [%p]\n", (word32)tmpGInOutSz, g);
    LogHex(g, tmpGInOutSz);

    releaseByteArray(env, input_buffer, input, 1);
    releaseByteArray(env, p_buffer, p, 1);
    releaseByteArray(env, g_buffer, g, 1);

#endif

    return ret;
}

JNIEXPORT int JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1init(
    JNIEnv *env, jclass class, jobject key_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* key = (ecc_key*) getNativeStruct(env, key_object);

    if (!key)
        return BAD_FUNC_ARG;

    ret = ecc_init(key);

    LogStr("ecc_init(key=%p) = %d\n", key, ret);

#endif

    return ret;

}

JNIEXPORT void JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1free(
    JNIEnv *env, jclass class, jobject key_object)
{
#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* key = (ecc_key*) getNativeStruct(env, key_object);

    if (key) {
        ecc_free(key);

        LogStr("ecc_free(key=%p)\n", key);
    }

#endif
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1make_1key(
    JNIEnv* env, jclass class, jobject rng_object, jint keysize,
    jobject key_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* key = (ecc_key*) getNativeStruct(env, key_object);
    RNG* rng = (RNG*) getNativeStruct(env, rng_object);

    if (!key || !rng)
        return BAD_FUNC_ARG;

    ret = ecc_make_key(rng, keysize, key);

    LogStr("ecc_make_key(rng=%p, keysize=%d, key=%p) = %d\n", rng, keysize, key,
        ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1shared_1secret__Lcom_wolfssl_wolfcrypt_Ecc_2Lcom_wolfssl_wolfcrypt_Ecc_2Ljava_nio_ByteBuffer_2_3J(
    JNIEnv* env, jclass class, jobject priv_object, jobject pub_object,
    jobject out_buffer, jlongArray outlen)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* priv = (ecc_key*) getNativeStruct(env, priv_object);
    ecc_key* pub = (ecc_key*) getNativeStruct(env, pub_object);
    byte* out = getDirectBufferAddress(env, out_buffer);
    word32 tmpOutLen;

    if (!priv || !pub || !out)
        return BAD_FUNC_ARG;

    (*env)->GetLongArrayRegion(env, outlen, 0, 1, (jlong*) &tmpOutLen);

    ret = ecc_shared_secret(priv, pub, out, &tmpOutLen);

    (*env)->SetLongArrayRegion(env, outlen, 0, 1, (jlong*) &tmpOutLen);

    LogStr("ecc_shared_secret(priv=%p, pub=%p, out, outLen) = %d\n", priv, pub,
        ret);
    LogStr("out[%u]: [%p]\n", (word32)tmpOutLen, out);
    LogHex(out, tmpOutLen);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1shared_1secret__Lcom_wolfssl_wolfcrypt_Ecc_2Lcom_wolfssl_wolfcrypt_Ecc_2_3B_3J(
    JNIEnv* env, jclass class, jobject priv_object, jobject pub_object,
    jbyteArray out_buffer, jlongArray outlen)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* priv = (ecc_key*) getNativeStruct(env, priv_object);
    ecc_key* pub = (ecc_key*) getNativeStruct(env, pub_object);
    byte* out = getByteArray(env, out_buffer);
    word32 tmpOutLen;

    LogStr("ecc_shared_secret(priv=%p, pub=%p, out, outLen) = %d\n", priv, pub,
        ret);

    if (!priv || !pub || !out)
        ret = BAD_FUNC_ARG;
    else {
        (*env)->GetLongArrayRegion(env, outlen, 0, 1, (jlong*) &tmpOutLen);

        ret = ecc_shared_secret(priv, pub, out, &tmpOutLen);

        (*env)->SetLongArrayRegion(env, outlen, 0, 1, (jlong*) &tmpOutLen);

        LogStr("out[%u]: [%p]\n", (word32)tmpOutLen, out);
        LogHex(out, tmpOutLen);
    }

    releaseByteArray(env, out_buffer, out, ret);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1import_1x963__Ljava_nio_ByteBuffer_2JLcom_wolfssl_wolfcrypt_Ecc_2(
    JNIEnv* env, jclass class, jobject in_buffer, jlong inLen,
    jobject key_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* key = (ecc_key*) getNativeStruct(env, key_object);
    byte* in = getDirectBufferAddress(env, in_buffer);

    if (!key || !in)
        return BAD_FUNC_ARG;

    ret = ecc_import_x963(in, inLen, key);

    LogStr("ecc_import_x963(in, inLen, key=%p) = %d\n", key, ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex(in, inLen);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1import_1x963___3BJLcom_wolfssl_wolfcrypt_Ecc_2(
    JNIEnv* env, jclass class, jbyteArray in_buffer, jlong inLen,
    jobject key_object)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* key = (ecc_key*) getNativeStruct(env, key_object);
    byte* in = getByteArray(env, in_buffer);

    ret = (!key || !in) ? BAD_FUNC_ARG
                        : ecc_import_x963(in, inLen, key);

    LogStr("ecc_import_x963(in, inLen, key=%p) = %d\n", key, ret);
    LogStr("in[%u]: [%p]\n", (word32)inLen, in);
    LogHex(in, inLen);

    releaseByteArray(env, in_buffer, in, 1);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1export_1x963__Lcom_wolfssl_wolfcrypt_Ecc_2Ljava_nio_ByteBuffer_2_3J(
    JNIEnv* env, jclass class, jobject key_object, jobject out_buffer,
    jlongArray outLen)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* key = (ecc_key*) getNativeStruct(env, key_object);
    byte* out = getDirectBufferAddress(env, out_buffer);
    word32 tmpOutLen;

    if (!key || !out)
        return BAD_FUNC_ARG;

    (*env)->GetLongArrayRegion(env, outLen, 0, 1, (jlong*) &tmpOutLen);

    ret = ecc_export_x963(key, out, &tmpOutLen);

    (*env)->SetLongArrayRegion(env, outLen, 0, 1, (jlong*) &tmpOutLen);

    LogStr("ecc_export_x963(key=%p, out, outLen) = %d\n", key, ret);
    LogStr("out[%u]: [%p]\n", (word32)tmpOutLen, out);
    LogHex(out, tmpOutLen);

#endif

    return ret;
}

JNIEXPORT jint JNICALL Java_com_wolfssl_wolfcrypt_Fips_ecc_1export_1x963__Lcom_wolfssl_wolfcrypt_Ecc_2_3B_3J(
    JNIEnv* env, jclass class, jobject key_object, jbyteArray out_buffer,
    jlongArray outLen)
{
    jint ret = NOT_COMPILED_IN;

#if defined(HAVE_FIPS) && defined(HAVE_ECC)

    ecc_key* key = (ecc_key*) getNativeStruct(env, key_object);
    byte* out = getByteArray(env, out_buffer);
    word32 tmpOutLen;

    LogStr("ecc_export_x963(key=%p, out, outLen) = %d\n", key, ret);

    if (!key || !out)
        ret = BAD_FUNC_ARG;
    else {
        (*env)->GetLongArrayRegion(env, outLen, 0, 1, (jlong*) &tmpOutLen);

        ret = ecc_export_x963(key, out, &tmpOutLen);

        (*env)->SetLongArrayRegion(env, outLen, 0, 1, (jlong*) &tmpOutLen);

        LogStr("out[%u]: [%p]\n", (word32)tmpOutLen, out);
        LogHex(out, tmpOutLen);
    }


    releaseByteArray(env, out_buffer, out, ret);

#endif

    return ret;
}
