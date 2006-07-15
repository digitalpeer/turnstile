/* $Id: crypto.h,v 1.2 2004/08/29 23:16:25 dp Exp $
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

/** @file crypto.h
@brief Crypto function definitions for interfacing with crypto++ library.
*/
#ifndef TURNSTILE_CRYPTO_H
#define TURNSTILE_CRYPTO_H

#include <string>

#include "libcrypto/filters.h"
#include "libcrypto/seckey.h"

class wxString;

namespace crypto
{

using namespace CryptoPP;
using namespace std;

string RandomString();

bool GenerateRSAKeys(unsigned int keyLength, string& privKey, string& pubKey);
string RSAEncryptString(const char* pubFilename, const char* message);
string RSADecryptString(const char* privFilename, const char* ciphertext);
string EncryptString(const char* instr, const char *passPhrase);
string DecryptString(const char* instr, const char *passPhrase);

/** Create a SHA hash of a string. */
wxString Sha1Hash(const wxString& message);

/* Get the MD5 hash of a file. */
wxString Md5Hash(const wxString& path);

string HexEncode(const string& in);
string HexDecode(const string& in);

#ifdef DEBUG
void runtest();
#endif

/** @class TwoFishCipher
@brief Little wrapper class to do TwoFish encrytion/decryption.
*/
class TwoFishCipher
{
public:

	TwoFishCipher();
	bool Init(const char* key/*, unsigned char *iv*/);
	const char* Encrypt(const char* seq);
	const char* Decrypt(const char* seq);
	std::string Key();
	~TwoFishCipher();

private:

	SymmetricCipher* cbcEncryption_;
	SymmetricCipher* cbcDecryption_;
	StreamTransformationFilter* cbcEncryptor_;
	StreamTransformationFilter* cbcDecryptor_;

	std::string strEncrypted_;
	std::string strDecrypted_;
	std::string key_;

};

}

#endif

