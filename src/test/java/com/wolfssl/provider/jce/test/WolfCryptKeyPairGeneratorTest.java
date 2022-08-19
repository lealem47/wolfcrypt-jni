/* wolfCryptKeyPairGeneratorTest.java
 *
 * Copyright (C) 2006-2022 wolfSSL Inc.
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

package com.wolfssl.provider.jce.test;

import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.BeforeClass;

import java.util.ArrayList;
import java.math.BigInteger;

import javax.crypto.KeyAgreement;
import javax.crypto.ShortBufferException;
import javax.crypto.spec.DHParameterSpec;

import java.security.Security;
import java.security.Provider;
import java.security.NoSuchProviderException;
import java.security.NoSuchAlgorithmException;
import java.security.InvalidKeyException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.AlgorithmParameters;
import java.security.AlgorithmParameterGenerator;
import java.security.SecureRandom;
import java.security.PublicKey;
import java.security.PrivateKey;
import java.security.KeyFactory;
import java.security.InvalidAlgorithmParameterException;
import java.security.spec.InvalidParameterSpecException;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.X509EncodedKeySpec;
import java.security.spec.PKCS8EncodedKeySpec;

import com.wolfssl.wolfcrypt.Ecc;
import com.wolfssl.wolfcrypt.test.Util;
import com.wolfssl.wolfcrypt.WolfCryptException;
import com.wolfssl.provider.jce.WolfCryptProvider;

public class WolfCryptKeyPairGeneratorTest {

    private static String supportedCurves[] = {
        "secp192r1",
        "prime192v2",
        "prime192v3",
        "prime239v1",
        "prime239v2",
        "prime239v3",
        "secp256r1",

        "secp112r1",
        "secp112r2",
        "secp128r1",
        "secp128r2",
        "secp160r1",
        "secp224r1",
        "secp384r1",
        "secp521r1",

        "secp160k1",
        "secp192k1",
        "secp224k1",
        "secp256k1",

        "brainpoolp160r1",
        "brainpoolp192r1",
        "brainpoolp224r1",
        "brainpoolp256r1",
        "brainpoolp320r1",
        "brainpoolp384r1",
        "brainpoolp512r1"
    };

    private static ArrayList<String> enabledCurves =
        new ArrayList<String>();

    private static int supportedKeySizes[] = {
        112, 128, 160, 192, 224, 239, 256, 320, 384, 521
    };

    private static ArrayList<Integer> enabledKeySizes =
        new ArrayList<Integer>();

    /* DH test params */
    private static byte[] prime = Util.h2b(
        "B0A108069C0813BA59063CBC30D5F500C14F44A7D6EF4AC625271CE8D" +
        "296530A5C91DDA2C29484BF7DB2449F9BD2C18AC5BE725CA7E791E6D4" +
        "9F7307855B6648C770FAB4EE02C93D9A4ADA3DC1463E1969D1174607A" +
        "34D9F2B9617396D308D2AF394D375CFA075E6F2921F1A7005AA048357" +
        "30FBDA76933850E827FD63EE3CE5B7C809AE6F50358E84CE4A00E9127" +
        "E5A31D733FC211376CC1630DB0CFCC562A735B8EFB7B0ACC036F6D9C9" +
        "4648F94090002B1BAA6CE31AC30B039E1BC246E4484E22736FC35FD49" +
        "AD6300748D68C90ABD4F6F1E348D3584BA6B9CD29BF681F084B63862F" +
        "5C6BD6B60665F7A6DC00676BBBC3A94183FBC7FAC8E21E7EAF003F93"
    );
    private static byte[] base = new byte[] { 0x02 };

    @BeforeClass
    public static void testProviderInstallationAtRuntime() {

        /* install wolfJCE provider at runtime */
        Security.addProvider(new WolfCryptProvider());

        Provider p = Security.getProvider("wolfJCE");
        assertNotNull(p);

        /* build list of enabled curves and key sizes,
         * getCurveSizeFromName() will return 0 if curve not found */
        Ecc tmp = new Ecc();
        for (int i = 0; i < supportedCurves.length; i++) {

            int size = tmp.getCurveSizeFromName(
                        supportedCurves[i].toUpperCase());

            if (size > 0) {
                enabledCurves.add(supportedCurves[i]);

                if (!enabledKeySizes.contains(Integer.valueOf(size))) {
                    enabledKeySizes.add(Integer.valueOf(size));
                }
            }
        }
    }

    @Test
    public void testGetKeyPairGeneratorFromProvider()
        throws NoSuchProviderException, NoSuchAlgorithmException {

        KeyPairGenerator kpg;
        kpg = KeyPairGenerator.getInstance("EC", "wolfJCE");

        /* getting a garbage algorithm should throw an exception */
        try {
            kpg = KeyPairGenerator.getInstance("NotValid", "wolfJCE");

            fail("KeyPairGenerator.getInstance should throw " +
                 "NoSuchAlgorithmException when given bad algorithm value");

        } catch (NoSuchAlgorithmException e) { }
    }

    @Test
    public void testKeyPairGeneratorEccInitializeWithParamSpec()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        /* try initializing KPG for all supported curves */
        for (int i = 0; i < enabledCurves.size(); i++) {

            KeyPairGenerator kpg =
                KeyPairGenerator.getInstance("EC", "wolfJCE");

            ECGenParameterSpec ecSpec =
                new ECGenParameterSpec(enabledCurves.get(i));
            kpg.initialize(ecSpec);

            /* bad curve should fail */
            try {
                ecSpec = new ECGenParameterSpec("BADCURVE");
                kpg.initialize(ecSpec);
            } catch (InvalidAlgorithmParameterException e) {}
        }
    }

    @Test
    public void testKeyPairGeneratorEccInitializeWithKeySize()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        /* try initializing KPG for all supported key sizes */
        for (int i = 0; i < enabledKeySizes.size(); i++) {

            KeyPairGenerator kpg =
                KeyPairGenerator.getInstance("EC", "wolfJCE");

            kpg.initialize(enabledKeySizes.get(i));

            /* bad key size should fail */
            try {
                kpg.initialize(9999);
            } catch (WolfCryptException e) {}
        }
    }

    @Test
    public void testKeyPairGeneratorEccKeyGenAllCurves()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        /* try generating keys for all supported curves */
        for (int i = 0; i < enabledCurves.size(); i++) {

            KeyPairGenerator kpg =
                KeyPairGenerator.getInstance("EC", "wolfJCE");

            ECGenParameterSpec ecSpec =
                new ECGenParameterSpec(enabledCurves.get(i));
            kpg.initialize(ecSpec);

            try {
                KeyPair kp = kpg.generateKeyPair();
            } catch (Exception e) {
                /* Some JDK versions' ECKeyFactory may not support all
                 * wolfCrypt's ECC curves */
                if (!e.toString().contains("Unknown named curve")) {
                    throw e;
                }
            }
        }
    }

    @Test
    public void testKeyPairGeneratorEccMultipleInits()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        if (enabledCurves.size() > 0) {

            KeyPairGenerator kpg =
                KeyPairGenerator.getInstance("EC", "wolfJCE");

            ECGenParameterSpec ecSpec =
                new ECGenParameterSpec(enabledCurves.get(0));

            kpg.initialize(ecSpec);
            kpg.initialize(ecSpec);
        }
    }

    @Test
    public void testKeyPairGeneratorEccMultipleKeyGen()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        if (enabledCurves.size() > 0) {

            KeyPairGenerator kpg =
                KeyPairGenerator.getInstance("EC", "wolfJCE");

            ECGenParameterSpec ecSpec =
                new ECGenParameterSpec(enabledCurves.get(0));
            kpg.initialize(ecSpec);

            KeyPair kp1 = kpg.generateKeyPair();
            KeyPair kp2 = kpg.generateKeyPair();
        }
    }

    @Test
    public void testKeyPairGeneratorEccNewKeyFromExisting()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException, InvalidKeySpecException {

        if (enabledCurves.size() > 0) {

            KeyPairGenerator kpg =
                KeyPairGenerator.getInstance("EC", "wolfJCE");

            ECGenParameterSpec ecSpec =
                new ECGenParameterSpec(enabledCurves.get(0));
            kpg.initialize(ecSpec);

            KeyPair kp = kpg.generateKeyPair();

            KeyFactory kf = KeyFactory.getInstance("EC");
            PublicKey pub = kf.generatePublic(new X509EncodedKeySpec(
                        kp.getPublic().getEncoded()));
            PrivateKey priv = kf.generatePrivate(new PKCS8EncodedKeySpec(
                        kp.getPrivate().getEncoded()));
        }
    }

    @Test
    public void testKeyPairGeneratorDhInitializeWithParamSpec()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        int testDHKeySizes[] = { 512, 1024, 2048 };

        for (int i = 0; i < testDHKeySizes.length; i++) {

            KeyPairGenerator kpg =
                KeyPairGenerator.getInstance("DH", "wolfJCE");

            DHParameterSpec spec = new DHParameterSpec(
                    new BigInteger(prime),
                    new BigInteger(base),
                    testDHKeySizes[i]);

            kpg.initialize(spec);
            KeyPair pair = kpg.generateKeyPair();
        }
    }

    @Test
    public void testKeyPairGeneratorDhInitWithKeySize()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        KeyPairGenerator kpg =
            KeyPairGenerator.getInstance("DH", "wolfJCE");

        try {
            kpg.initialize(512);
        } catch (RuntimeException e) {
            /* expected, users need to explicitly set DH params */
        }
    }

    @Test
    public void testKeyPairGeneratorDhMultipleInits()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        KeyPairGenerator kpg =
            KeyPairGenerator.getInstance("DH", "wolfJCE");

        DHParameterSpec spec = new DHParameterSpec(
                new BigInteger(prime), new BigInteger(base), 512);

        kpg.initialize(spec);
        kpg.initialize(spec);
    }

    @Test
    public void testKeyPairGeneratorDhMultipleKeyGen()
        throws NoSuchProviderException, NoSuchAlgorithmException,
               InvalidAlgorithmParameterException {

        KeyPairGenerator kpg =
            KeyPairGenerator.getInstance("DH", "wolfJCE");

        DHParameterSpec spec = new DHParameterSpec(
                new BigInteger(prime), new BigInteger(base), 512);

        kpg.initialize(spec);

        KeyPair kp1 = kpg.generateKeyPair();
        KeyPair kp2 = kpg.generateKeyPair();
    }
}

