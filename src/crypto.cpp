/* $Id: crypto.cpp,v 1.2 2004/07/02 23:24:22 dp Exp $
 *
 * Turnstile
 *
 * Copyright (C) 2004, J.D. Henderson <www.digitalpeer.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/** @file crypto.cpp
@brief Crypto function implimentations.
*/
#include <wx/string.h>

#include "crypto.h"

#include <iostream>
#include <ctime>
#include <string>

#include "libcrypto/md5.h"
#include "libcrypto/sha.h"
#include "libcrypto/files.h"
#include "libcrypto/rng.h"
#include "libcrypto/hex.h"
#include "libcrypto/gzip.h"
#include "libcrypto/default.h"
#include "libcrypto/rsa.h"
#include "libcrypto/randpool.h"
#include "libcrypto/ida.h"
#include "libcrypto/base64.h"
#include "libcrypto/rsa.h"
#include "libcrypto/twofish.h"
#include "libcrypto/blowfish.h"

#include <iostream>
#include <ctime>
#include <iomanip>

#if (_MSC_VER >= 1000)
#include <crtdbg.h>		// for the debug heap
#endif

namespace crypto
{

using namespace CryptoPP;
using namespace std;

RandomPool& GlobalRNG()
{
	static RandomPool randomPool;
	return randomPool;
}

string RandomString()
{
	const unsigned int SIZE = 32;
	byte answer[SIZE];
	RandomPool().GenerateBlock(answer,SIZE);
	return string((char*)answer,SIZE);
}

string
RSAEncryptString(const char *pubFilename, const char* message)
{
	StringSource pubFile(pubFilename, true, new HexDecoder);
	RSAES_OAEP_SHA_Encryptor pub(pubFile);

	string result;
	StringSource(message, true, new PK_EncryptorFilter(GlobalRNG(), pub, new HexEncoder(new StringSink(result))));
	return result;
}

string
RSADecryptString(const char *privFilename, const char* ciphertext)
{
	StringSource privFile(privFilename, true, new HexDecoder);
	RSAES_OAEP_SHA_Decryptor priv(privFile);

	string result;
	StringSource(ciphertext, true, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(result))));
	return result;
}

bool
GenerateRSAKeys(unsigned int keyLength, string& privKey, string& pubKey)
{
	try
	{
		RSAES_OAEP_SHA_Decryptor priv(GlobalRNG(), keyLength);
		HexEncoder privFile(new StringSink(privKey));
		priv.DEREncode(privFile);
		privFile.MessageEnd();

		RSAES_OAEP_SHA_Encryptor pub(priv);
		HexEncoder pubFile(new StringSink(pubKey));
		pub.DEREncode(pubFile);
		pubFile.MessageEnd();
	}
	catch (...)
	{
		return false;
	}

	return true;
}

#ifdef DEBUG
void runtest()
{
	const char* message = "src-agent:TURNSTILE/0.0.1^src-nid:D7C6CF5BAA93B5535F7E7026731530A8B260E3CE^src-addr:dp3/5001^assign-nid:48AD3A178D10C34EC3824A92BBC6844AF162F2EE^nid:D7C6CF5BAA93B5535F7E7026731530A8B260E3CE^node:dp3/5001^";

	//const char* message = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

	string pubKey;
	string privKey;

	GenerateRSAKeys(2048, privKey, pubKey);

	cout << "pub key:" << pubKey << endl;
	cout << "priv key:" << privKey << endl;

	string encrypted = RSAEncryptString(pubKey.c_str(),message);
	string decrypted = RSADecryptString(privKey.c_str(), encrypted.c_str());
	cout << "encrypted text:" << encrypted << endl;
	cout << "decrypted text:" << decrypted  << endl;

	/*const char* key = "helloworld";

	string encrypted = EncryptString(message,key);
	string decrypted = DecryptString(encrypted.c_str(),key);
	cout << "encrypted text:" << encrypted << endl;
	cout << "decrypted text:" << decrypted  << endl;
*/

}
#endif

string
EncryptString(const char *instr, const char *passPhrase)
{
	string outstr;

	DefaultEncryptorWithMAC encryptor(passPhrase, new HexEncoder(new StringSink(outstr)));
	encryptor.Put((byte *)instr, strlen(instr));
	encryptor.MessageEnd();

	return outstr;
}

string
DecryptString(const char *instr, const char *passPhrase)
{
	string outstr;

	HexDecoder decryptor(new DefaultDecryptorWithMAC(passPhrase, new StringSink(outstr)));
	decryptor.Put((byte *)instr, strlen(instr));
	decryptor.MessageEnd();

	return outstr;
}

string
HexEncode(const string& in)
{
	string answer;
   	StringSource(in, true, new HexEncoder(new StringSink(answer)));
   	return answer;
}

string
HexDecode(const string& in)
{
	string answer;
   	StringSource(in, true, new HexDecoder(new StringSink(answer)));
   	return answer;
}

wxString
Md5Hash(const wxString& path)
{
	//output size of the buffer (*2 for hex)
	const int REAL_DIGEST_SIZE = 2 * MD5::DIGESTSIZE;

	MD5 hash;
	byte buffer[REAL_DIGEST_SIZE];

	FileSource f(path.c_str(), true,
			new HashFilter(hash,
			new HexEncoder(new ArraySink(buffer,REAL_DIGEST_SIZE))));

	return wxString((const char*)buffer,REAL_DIGEST_SIZE);
}

wxString
Sha1Hash(const wxString& message)
{
	//output size of the buffer (double for hex)
	const int REAL_DIGEST_SIZE = 2 * SHA1::DIGESTSIZE;
	SHA1 hash;
	byte buffer[REAL_DIGEST_SIZE];

	StringSource f(message.c_str(), true,
			new HashFilter(hash,
			new HexEncoder(new ArraySink(buffer,REAL_DIGEST_SIZE))));

	return wxString((const char*)buffer,REAL_DIGEST_SIZE);
}

TwoFishCipher::TwoFishCipher()
	: cbcEncryption_(0),cbcDecryption_(0),cbcEncryptor_(0),cbcDecryptor_(0)
{;}

bool
TwoFishCipher::Init(const char* key/*, unsigned char *iv*/)
{
	try
	{

	const byte iv[] = {0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef};

	key_ = string(key,strlen(key));
/*
	cbcEncryption_ = new CFB_Mode<Rijndael>::Encryption((byte*)key,(unsigned int)strlen(key),iv);
	cbcEncryptor_ = new StreamTransformationFilter(*((SymmetricCipher*)cbcEncryption_),
				new StringSink(strEncrypted_),
				StreamTransformationFilter::NO_PADDING);

	cbcDecryption_ = new CFB_Mode<Rijndael>::Decryption((byte*)key,(unsigned int)strlen(key),iv);
	cbcDecryptor_ = new StreamTransformationFilter(*((SymmetricCipher*)cbcDecryption_),
				new StringSink(strDecrypted_),
				StreamTransformationFilter::NO_PADDING);
*/
	}
	catch (...)
	{
		return false;
	}

	return true;
}

const char*
TwoFishCipher::Encrypt(const char* seq)
{
	strEncrypted_.erase();
	cbcEncryptor_->Put((byte*)seq,(unsigned int)strlen(seq));
	cbcEncryptor_->MessageEnd();
	return strEncrypted_.c_str();
}

const char*
TwoFishCipher::Decrypt(const char* seq)
{
	strDecrypted_.erase();
	cbcDecryptor_->Put((byte*)seq,(unsigned int)strlen(seq));
	cbcDecryptor_->MessageEnd();
	return strDecrypted_.c_str();
}

TwoFishCipher::~TwoFishCipher()
{
	if (cbcEncryption_ != 0)
		delete cbcEncryption_;
	if (cbcDecryption_ != 0)
		delete cbcDecryption_;
	if (cbcEncryptor_ != 0)
		delete cbcEncryptor_;
	if (cbcDecryptor_ != 0)
		delete cbcDecryptor_;
}

std::string
TwoFishCipher::Key()
{
	return key_;
}

}

/*
byte* iv = new byte[key.length()];
std::string ciphertext;
//ciphertext.clear();

CryptoPP::BlowfishEncryption
blowEncrypt((byte*)key.c_str(),key.length());
CryptoPP::CBCPaddedEncryptor cbcEncryptor(
	blowEncrypt,iv,new CryptoPP::StringSink(ciphertext));
cbcEncryptor.Put((byte*)stringToEncrypt.c_str(),stringToEncrypt.length());
cbcEncryptor.MessageEnd();

std::string hexCipherText;
CryptoPP::HexEncoder output(new
CryptoPP::StringSink(hexCipherText));
output.Put((byte*)ciphertext.c_str(),ciphertext.length());

delete iv;
*/

#if 0
char* rsaSign( char *inPrivateKey,
                            unsigned char *inData, int inDataLength ) {

    HexDecoder privateDecoder;
    privateDecoder.Put( (byte *)inPrivateKey, strlen( inPrivateKey ) );
    privateDecoder.MessageEnd();

    try {
        RSASSA_PKCS1v15_SHA_Signer priv( privateDecoder );

        // RSASSA_PKCS1v15_SHA_Signer ignores the rng.
        // Use a real RNG for other signature schemes!
        NullRNG rng;

        HexEncoder *signatureEncoder = new HexEncoder();

        // filter will push data through into hex encoder
        SignerFilter signatureFilter( rng, priv, signatureEncoder );

        signatureFilter.Put( (byte *)inData, inDataLength );

        // unlimited propagation... propagates to hex encoder
        signatureFilter.MessageEnd( -1 );


        // extract signature from hex encoder
        int sigLength = signatureEncoder->MaxRetrievable();
        char *signature = new char[ sigLength + 1 ];
        signatureEncoder->Get( (byte *)signature, sigLength );
        
        signature[ sigLength ] = '\0';

        return signature;        
        }
    catch( Exception inException ) {
        // bad key
        return NULL;
        }
    }



char rsaVerify( char *inPublicKey,
                             unsigned char *inData, int inDataLength,
                             char *inSignature ) {

    HexDecoder publicEncoder;
    publicEncoder.Put( (byte *)inPublicKey, strlen( inPublicKey ) );
    publicEncoder.MessageEnd();
    
    try {                   
        RSASSA_PKCS1v15_SHA_Verifier pub( publicEncoder );

        
        HexDecoder signatureDecoder;
        signatureDecoder.Put( (byte *)inSignature, strlen( inSignature ) );
        signatureDecoder.MessageEnd();

        if( signatureDecoder.MaxRetrievable() != pub.SignatureLength() ) {
            return false;
            }
    
        SecByteBlock signature( pub.SignatureLength() );
        signatureDecoder.Get( signature, signature.size );

        VerifierFilter verifierFilter( pub );
        verifierFilter.PutSignature( signature );

        verifierFilter.Put( (byte *)inData, inDataLength );
        // unlimited propagation
        verifierFilter.MessageEnd( -1 );

        
        byte result = 0;
        verifierFilter.Get( result );
        return ( result == 1 );
        }
    catch( Exception inException ) {
        // bad key
        return false;
        }
    }

}

void Base64EncodeFile(const char *in, const char *out)
{
	FileSource(in, true, new Base64Encoder(new FileSink(out)));
}

void Base64DecodeFile(const char *in, const char *out)
{
	FileSource(in, true, new Base64Decoder(new FileSink(out)));
}

void GzipFile(const char *in, const char *out, int deflate_level)
{
	EqualityComparisonFilter comparison;

	Gunzip gunzip(new ChannelSwitch(comparison, "0"));
	gunzip.SetAutoSignalPropagation(0);

	FileSink sink(out);

	ChannelSwitch *cs;
	Gzip gzip(cs = new ChannelSwitch(sink), deflate_level);
	cs->AddDefaultRoute(gunzip);

	cs = new ChannelSwitch(gzip);
	cs->AddDefaultRoute(comparison, "1");
	FileSource source(in, true, cs);

	comparison.ChannelMessageSeriesEnd("0");
	comparison.ChannelMessageSeriesEnd("1");
}

void GunzipFile(const char *in, const char *out)
{
	FileSource(in, true, new Gunzip(new FileSink(out)));
}
#endif


