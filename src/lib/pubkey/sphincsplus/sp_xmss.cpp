/*
* Sphincs+ XMSS logic
* (C) 2023 Jack Lloyd
*     2023 Fabian Albert, René Meusel, Amos Treiber - Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
**/

#include <botan/internal/sp_xmss.h>

namespace Botan
{

SphincsXmssRootNode xmss_sign(std::span<uint8_t> out_sig,
                              const SphincsXmssRootNode& root,
                              const SphincsPublicSeed& public_seed,
                              const SphincsSecretSeed& secret_seed,
                              Sphincs_Address& wots_addr,
                              Sphincs_Address& tree_addr,
                              uint32_t idx_leaf,
                              const Sphincs_Parameters& params,
                              Sphincs_Hash_Functions& hashes)
   {
   auto auth_path_location = out_sig.subspan(params.wots_bytes(), out_sig.size() - params.wots_bytes());

   auto steps = chain_lengths(root, params);

   Sphincs_Address leaf_addr = Sphincs_Address::as_subtree_from(wots_addr);
   Sphincs_Address pk_addr = Sphincs_Address::as_subtree_from(wots_addr);

   tree_addr.set_type(Sphincs_Address_Type::HashTree);
   pk_addr.set_type(Sphincs_Address_Type::WotsPublicKeyCompression);

   GenerateLeafFunction wots_gen_leaf = std::bind(wots_gen_leaf_spec,
                                                   out_sig,
                                                   std::placeholders::_1,
                                                   std::ref(secret_seed),
                                                   std::ref(public_seed),
                                                   std::placeholders::_2,
                                                   std::ref(idx_leaf),
                                                   std::ref(steps),
                                                   std::ref(leaf_addr),
                                                   std::ref(pk_addr),
                                                   std::ref(params),
                                                   std::ref(hashes));

   SphincsXmssRootNode next_root(params.n());
   treehash_spec(next_root, auth_path_location, params,
               hashes, public_seed,
               idx_leaf,
               0, params.xmss_tree_height(), wots_gen_leaf,
               tree_addr);

   return next_root;
   }

SphincsXmssRootNode xmss_gen_root(const Sphincs_Parameters& params,
                                  const SphincsPublicSeed& public_seed,
                                  const SphincsSecretSeed& secret_seed,
                                  Sphincs_Hash_Functions& hashes)
   {
   /* We do not need the a sig/auth path in key generation, but it simplifies the
         code to have just one treehash routine that computes both root and path
         in one function. */
   std::vector<uint8_t> dummy_sig(params.xmss_tree_height() * params.n() + params.wots_bytes());
   SphincsXmssRootNode dummy_root(params.n());

   Sphincs_Address top_tree_addr(Sphincs_Address_Type::HashTree);
   Sphincs_Address wots_addr(Sphincs_Address_Type::WotsPublicKeyCompression);

   top_tree_addr.set_layer(params.d() - 1);
   wots_addr.set_layer(params.d() - 1);

   SphincsXmssRootNode root = xmss_sign(dummy_sig,
                                        dummy_root,
                                        public_seed,
                                        secret_seed,
                                        wots_addr, top_tree_addr,
                                        static_cast<uint32_t>(~0) /* ~0 means "don't bother generating a sig/auth path */ ,
                                        params, hashes);

   return root;
   }

}