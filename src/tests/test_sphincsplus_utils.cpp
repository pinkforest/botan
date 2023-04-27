/*
* (C) 2023 Jack Lloyd
*     2023 Fabian Albert, René Meusel - Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "tests.h"

#if defined(BOTAN_HAS_SPHINCS_PLUS) && defined (BOTAN_HAS_SHA2_32)

#include <botan/internal/sp_address.h>
#include <botan/hex.h>

namespace Botan_Tests {

namespace {

std::vector<Test::Result> test_sphincsplus_address()
   {
   auto sha256 = [](const Botan::Sphincs_Address& adrs)
      {
      auto h = Botan::HashFunction::create_or_throw("SHA-256");
      adrs.apply_to_hash(*h);
      return h->final_stdvec();
      };

   return
      {
      Botan_Tests::CHECK("default address", [&](Test::Result& result)
         {
         Botan::Sphincs_Address a;
         result.test_is_eq("SHA-256(32*0x00)", sha256(a), Botan::hex_decode("66687aadf862bd776c8fc18b8e9f8e20089714856ee233b3902a591d0d5f2925"));
         }),

      Botan_Tests::CHECK("set up an address", [&](Test::Result& result)
         {
         Botan::Sphincs_Address a;
         a.set_layer(1337)
          .set_tree(4294967338 /* longer than 32bits */)
          .set_type(Botan::Sphincs_Address::ForsTree)
          .set_keypair(131072)
          .set_chain(67108864)
          .set_hash(1073741824);
         result.test_is_eq("SHA-256(a1)", sha256(a), Botan::hex_decode("607fdc9d063168fbea64e4da2a255693314712d859062abb80cf7c78116ded2a"));
         }),

      Botan_Tests::CHECK("set up another address", [&](Test::Result& result)
         {
         Botan::Sphincs_Address a;
         a.set_layer(1337)
          .set_tree(4294967338 /* longer than 32bits */)
          .set_type(Botan::Sphincs_Address::ForsTree)
          .set_keypair(131072)
          .set_tree_height(67108864)
          .set_tree_index(1073741824);
         result.test_is_eq("SHA-256(a2)", sha256(a), Botan::hex_decode("607fdc9d063168fbea64e4da2a255693314712d859062abb80cf7c78116ded2a"));
         }),

      Botan_Tests::CHECK("copy subtree", [&](Test::Result& result)
         {
         Botan::Sphincs_Address a;
         a.set_layer(1337)
          .set_tree(4294967338 /* longer than 32bits */)
          .set_type(Botan::Sphincs_Address::ForsTree)
          .set_keypair(131072)
          .set_tree_height(67108864)
          .set_tree_index(1073741824);

         auto subtree1 = Botan::Sphincs_Address::as_subtree_from(a);
         Botan::Sphincs_Address subtree2;
         subtree2.copy_subtree_from(a);

         result.test_is_eq("SHA-256(subtree1)", sha256(subtree1), Botan::hex_decode("f192c8f8e946aa16d16eafe88bd4eabcc88a305b69bb7c0bb49e65bd122bb973"));
         result.test_is_eq("SHA-256(subtree2)", sha256(subtree2), Botan::hex_decode("f192c8f8e946aa16d16eafe88bd4eabcc88a305b69bb7c0bb49e65bd122bb973"));
         }),

      Botan_Tests::CHECK("copy keypair", [&](Test::Result& result)
         {
         Botan::Sphincs_Address a;
         a.set_layer(1337)
          .set_tree(4294967338 /* longer than 32bits */)
          .set_type(Botan::Sphincs_Address::ForsTree)
          .set_keypair(131072)
          .set_chain(67108864)
          .set_hash(1073741824);

         auto keypair1 = Botan::Sphincs_Address::as_keypair_from(a);
         Botan::Sphincs_Address keypair2;
         keypair2.copy_keypair_from(a);

         result.test_is_eq("SHA-256(keypair1)", sha256(keypair1), Botan::hex_decode("1cdd4835a6057306678e7d8cb903c140aba1d4805a8a1f75b11f1129bb22d08c"));
         result.test_is_eq("SHA-256(keypair2)", sha256(keypair2), Botan::hex_decode("1cdd4835a6057306678e7d8cb903c140aba1d4805a8a1f75b11f1129bb22d08c"));
         }),
      };
   }

}  // namespace

BOTAN_REGISTER_TEST_FN("pubkey", "sphincsplus_address", test_sphincsplus_address);

}  // namespace Botan_Tests

#endif  // BOTAN_HAS_SPHINCS_PLUS