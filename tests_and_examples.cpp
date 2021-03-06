#include "RSAES.hpp"
#include <iostream>  // showing results of tests
#include <random>    // Fast/easy randomness for generating test words

std::random_device rd;
std::mt19937 mt(rd());

std::string word_bank[50] = {
			     "pleasant",
			     "foot",
			     "elfin",
			     "calendar",
			     "settle",
			     "size",
			     "trip",
			     "float",
			     "sand",
			     "good",
			     "stain",
			     "trite",
			     "colorful",
			     "street",
			     "dusty",
			     "range",
			     "blot",
			     "direction",
			     "cent",
			     "white",
			     "angry",
			     "sack",
			     "wave",
			     "weather",
			     "stitch",
			     "ritzy",
			     "scintill",
			     "deliver",
			     "synonymo",
			     "quicksan",
			     "cub",
			     "sofa",
			     "callous",
			     "disagree",
			     "dashing",
			     "daughter",
			     "jar",
			     "sniff",
			     "ear",
			     "powder",
			     "wait",
			     "shame",
			     "needy",
			     "dreary",
			     "x-ray",
			     "labored",
			     "can",
			     "incompet",
			     "pricey",
			     "jagged"
};
std::uniform_int_distribution<unsigned short> dist_50(0, 49);

std::string test_high_level(){
  try{
    std::cout << "Start encryption manager 1 and grab the RSA public key:" << std::endl;
    RSAES::EncryptionManager Bob(2048);
    std::string msg = Bob.getPublicKey();
    std::cout << msg << std::endl << std::endl;

    std::cout << "Start encryption manager 2, generate a random AES key, and send it back encrypted over RSA:" << std::endl;
    RSAES::EncryptionManager Allice(msg);
    msg = Allice.getKeyResponse();
    std::cout << msg << std::endl << std::endl;

    std::cout << "Register the AES key with manager 1. Now we can send a message:" << std::endl;
    Bob.registerPass(msg);
    std::string msg_s = word_bank[dist_50(mt)];
    int words = dist_50(mt);
    for(int i=0; i<words; ++i)
      (msg_s+=' ')+=word_bank[dist_50(mt)];
    msg = Bob.encrypt(msg_s);
    std::cout << msg << std::endl << std::endl;

    std::cout << "Now we can decrypt it using manager 2:" << std::endl;
    msg = Allice.decrypt(msg);
    std::cout << msg << std::endl << std::endl;
    if(msg!=msg_s)
      throw std::runtime_error("Messages aren't same");

    std::cout << "Pack up both objects as if we're saving data for a later session" << std::endl << std::endl;
    std::string Bob_pack = Bob.pack();
    std::string Allice_pack = Allice.pack();
    Bob.destroy();
    Allice.destroy();
    std::cout << "For reference, here's the packed strings:" << std::endl << Bob_pack << std::endl << std::endl;

    std::cout << "Unpack the managers for further use" << std::endl << std::endl;
    RSAES::EncryptionManager Bob2, Allice2;
    Bob2.unpack(Bob_pack);
    Allice2.unpack(Allice_pack);
    std::cout << "Re-packed string:" << std::endl << Bob2.pack() << std::endl;
    std::cout << "Now let's go the other way. Encrypt with manager 2:" << std::endl;
    msg_s = word_bank[dist_50(mt)];
    words = dist_50(mt);
    for(int i=0; i<words; ++i)
      (msg_s+=' ')+=word_bank[dist_50(mt)];
    msg = Allice2.encrypt(msg_s);
    std::cout << msg << std::endl << std::endl;

    std::cout << "And decrypt with manager 1:" << std::endl;
    msg = Bob2.decrypt(msg);
    std::cout << msg << std::endl << std::endl;
    if(msg!=msg_s)
      throw std::runtime_error("Messages aren't same");
  } catch (const std::runtime_error& error){
    return error.what();
  }
  return "";
}

std::string test_low_level_RSA(){
  try{
    std::cout << "Start an RSAmanager and grab the public key:" << std::endl;
    RSAES::RSA::RSAmanager Bob(1024);
    std::string keyStr = RSAES::RSA::packKey(Bob.public_key);
    std::cout << keyStr << std::endl << std::endl;

    std::cout << "Encrypt a message using the public key:" << std::endl;
    std::string msg_s = word_bank[dist_50(mt)];
    int words = dist_50(mt)/2;
    for(int i=0; i<words; ++i)
      (msg_s+=' ')+=word_bank[dist_50(mt)];
    std::pair<mpz_t,mpz_t> *recvKey;
    RSAES::RSA::unpackKey(&recvKey, keyStr.c_str());
    std::string msg = RSAES::RSA::encrypt(msg_s, recvKey);
    mpz_clear(recvKey->first);
    mpz_clear(recvKey->second);
    delete recvKey;
    std::cout << msg << std::endl << std::endl;

    std::cout << "Decrypt the message using the private key:" << std::endl;
    msg = Bob.decrypt(msg);
    std::cout << msg << std::endl << std::endl;
    if(msg_s!=msg)
      throw std::runtime_error("Messages aren't same");
  } catch (const std::runtime_error& error){
    return error.what();
  }
  return "";
}

std::string test_low_level_AES(){
  try{
    std::cout << "Start an AESkey and encrypt a message at 128 bits key size" << std::endl;
    RSAES::AES::AESkey Bob(128);
    std::string msg_s = word_bank[dist_50(mt)];
    int words = dist_50(mt);
    for(int i=0; i<words; ++i)
      (msg_s+=' ')+=word_bank[dist_50(mt)];
    std::string msg = RSAES::AES::big_encrypt(msg_s, Bob);
    std::cout << msg << std::endl << std::endl;

    std::cout << "Decrypt the message using the key" << std::endl;
    msg = RSAES::AES::big_decrypt(msg, Bob);
    std::cout << msg << std::endl << std::endl;
    {
      std::cout << "For reference, the key in base64:" << std::endl;
      auto p = Bob.expanded_key.begin();
      std::vector<unsigned char> exp(p, p+Bob.base); // send the un-expanded key so that we need less data to send
      std::string AES_string(exp.begin(), exp.end());
      size_t key_s;
      unsigned char* prt = RSAES::UTIL::base64_encode((const unsigned char*)AES_string.c_str(), AES_string.size(), &key_s);
      for(size_t i=0; i<key_s; ++i)
	std::cout << prt[i];
      free(prt);
      putchar('\n');
      putchar('\n');
    }
    if(msg_s!=msg)
      throw std::runtime_error("Messages aren't same - small");



    
    std::cout << "Start an AESkey and encrypt a message at 1 megabit key size" << std::endl;
    RSAES::AES::AESkey Allice(1048576);
    msg_s = word_bank[dist_50(mt)];
    words = dist_50(mt);
    for(int i=0; i<words; ++i)
      (msg_s+=' ')+=word_bank[dist_50(mt)];
    msg = RSAES::AES::big_encrypt(msg_s, Allice);
    std::cout << msg << std::endl << std::endl;

    std::cout << "Decrypt the message using the key" << std::endl;
    msg = RSAES::AES::big_decrypt(msg, Allice);
    std::cout << msg << std::endl << std::endl;
    if(msg_s!=msg)
      throw std::runtime_error("Messages aren't same - small");
  } catch (const std::runtime_error& error){
    return error.what();
  }
  return "";
}

bool test_gigabit(){
  try{
    std::cout << "Create and expand an AESkey of 1 gigabit key size" << std::endl;
    RSAES::AES::AESkey Allice(1073741824);
    std::cout << "Key created. Encrypt the message using the key" << std::endl;
    std::string msg_s = word_bank[dist_50(mt)];

    for(int i=0; i<50; ++i)
      (msg_s+=' ')+=word_bank[dist_50(mt)];
    std::string msg = RSAES::AES::big_encrypt(msg_s, Allice);
    std::cout << msg << std::endl << std::endl;

    std::cout << "Decrypt the message using the key" << std::endl;
    msg = RSAES::AES::big_decrypt(msg, Allice);
    std::cout << msg << std::endl << std::endl;
    if(msg_s!=msg)
      throw std::runtime_error("Messages aren't same - gigabit");
  } catch (const std::runtime_error& error){
    return false;
  }
  return true;
}


int main(){
  bool s_gigabit = true;
  unsigned int s_aes=0, f_aes=0, f_high=0, s_high=0, f_rsa=0, s_rsa=0;
  bool
    test_high = true
    ,test_rsa = false
    ,test_aes = false
    ,test_gig = false
    ;

  if(test_high){
    std::cout << "TESTING HIGH LEVEL INTERFACE" << std::endl;
    for(unsigned int done=1; done; ++done){
      std::string result = test_high_level();
      if(result=="")
	++s_high;
      else {
	++f_high;
	std::cout << "TEST FAILED WITH MESSAGE: " << std::endl << result << std::endl;
	exit(0);
      }
      std::cout << "-----------------------" << std::endl;
      std::cout << "HIGH LEVEL INTERFACE" << std::endl;
      std::cout << "SUCESSFULL TESTS: " << s_high << std::endl;
      std::cout << "FAILED TESTS: " << f_high << std::endl;
      std::cout << "-----------------------" << std::endl;
    }
  }

  if(test_rsa){
    std::cout << "TESTING LOW LEVEL INTERFACE - RSA" << std::endl;
    for(unsigned int done=0; done<10; ++done){
      std::string result = test_low_level_RSA();
      if(result=="")
	++s_rsa;
      else {
	++f_rsa;
	std::cout << "TEST FAILED WITH MESSAGE: " << std::endl << result << std::endl;
      }
      std::cout << "-----------------------" << std::endl;
      std::cout << "LOW LEVEL INTERFACE RSA" << std::endl;
      std::cout << "SUCESSFULL TESTS: " << s_rsa << std::endl;
      std::cout << "FAILED TESTS: " << f_rsa << std::endl;
      std::cout << "-----------------------" << std::endl;
    }
  }

  if(test_aes){
    std::cout << "TESTING LOW LEVEL INTERFACE - AES" << std::endl;
    for(unsigned int done=0; done<10; ++done){
      std::string result = test_low_level_AES();
      if(result=="")
	++s_aes;
      else {
	++f_aes;
	std::cout << "TEST FAILED WITH MESSAGE: " << std::endl << result << std::endl;
      }
      std::cout << "-----------------------" << std::endl;
      std::cout << "LOW LEVEL INTERFACE AES" << std::endl;
      std::cout << "SUCESSFULL TESTS: " << s_aes << std::endl;
      std::cout << "FAILED TESTS: " << f_aes << std::endl;
      std::cout << "-----------------------" << std::endl;
    }
  }

  if(test_gig){
    std::cout << "TESTING LOW LEVEL INTERFACE - AES AT 1 GIGABIT KEY SIZE" << std::endl;
    s_gigabit = test_gigabit();
    if(!s_gigabit)
      std::cout << "TEST FAILED WITH MESSAGE: " << std::endl << "Messages aren't same - gigabit" << std::endl;
    std::cout << "-----------------------" << std::endl;
    std::cout << "GIGABIT AES LOW LEVEL  " << std::endl;
    std::cout << "TEST " << (s_gigabit?"PASSED":"FAILED") << std::endl;
    std::cout << "-----------------------" << std::endl;
  }
  
  std::cout << std::endl << "TEST RESULTS:" << std::endl;
  std::cout << "-----------------------" << std::endl;
  if(test_high){
    std::cout << "HIGH LEVEL INTERFACE" << std::endl;
    std::cout << "SUCESSFULL TESTS: " << s_high << std::endl;
    std::cout << "FAILED TESTS: " << f_high << std::endl;
    std::cout << "-----------------------" << std::endl;
  }
  if(test_rsa){
    std::cout << "LOW LEVEL INTERFACE RSA" << std::endl;
    std::cout << "SUCESSFULL TESTS: " << s_rsa << std::endl;
    std::cout << "FAILED TESTS: " << f_rsa << std::endl;
    std::cout << "-----------------------" << std::endl;
  }
  if(test_aes){
    std::cout << "LOW LEVEL INTERFACE AES" << std::endl;
    std::cout << "SUCESSFULL TESTS: " << s_aes << std::endl;
    std::cout << "FAILED TESTS: " << f_aes << std::endl;
    std::cout << "-----------------------" << std::endl;
  }
  if(test_gig){
    std::cout << "GIGABIT AES LOW LEVEL  " << std::endl;
    std::cout << "TEST " << (s_gigabit?"PASSED":"FAILED") << std::endl;
    std::cout << "-----------------------" << std::endl;
  }
  if(!f_high && !f_rsa && !f_aes && s_gigabit)
    std::cout << "PASSED TESTS" << std::endl;
  else
    std::cout << "FAILED TESTS" << std::endl;
  
  return 0;
}
