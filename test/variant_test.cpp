#include <boost/test/unit_test.hpp>
#include "variant/variant_reader.h"
#include "variant/variant.h"
#include "variant/variant_builder.h"
#include "missing.h"
#include "utils/variant_utils.h"

using namespace std;
using namespace gamgee;
using namespace boost;

BOOST_AUTO_TEST_CASE( allele_mask_simple_snp ) {
  const auto rec = (*(SingleVariantReader("testdata/test_variants.bcf").begin())); // take the first record
  const auto am = rec.allele_mask();
  BOOST_REQUIRE_EQUAL(am.size(), 2u);
  BOOST_CHECK(am[0] == AlleleType::REFERENCE);
  BOOST_CHECK(am[1] == AlleleType::SNP);
}

BOOST_AUTO_TEST_CASE( allele_mask_simple_insertion ) {
  auto it = SingleVariantReader("testdata/test_variants.bcf").begin(); 
  ++it; ++it; ++it; // take the fourth record (it is an insertion)
  const auto rec = *it;
  const auto am = rec.allele_mask();
  BOOST_REQUIRE_EQUAL(am.size(), 2u);
  BOOST_CHECK(am[0] == AlleleType::REFERENCE);
  BOOST_CHECK(am[1] == AlleleType::INSERTION);
}

BOOST_AUTO_TEST_CASE( allele_mask_simple_deletion ) {
  auto it = SingleVariantReader("testdata/test_variants.bcf").begin(); 
  ++it; ++it; // take the third record -- this would be so much simpler with a VariantBuilder....
  const auto rec = *it;
  const auto am = rec.allele_mask();
  BOOST_REQUIRE_EQUAL(am.size(), 2u);
  BOOST_CHECK(am[0] == AlleleType::REFERENCE);
  BOOST_CHECK(am[1] == AlleleType::DELETION);
}

BOOST_AUTO_TEST_CASE( allele_mask_snp_and_insertion ) {
  auto it = SingleVariantReader("testdata/test_variants.bcf").begin(); 
  ++it; ++it; ++it; ++it; // take the third record -- this would be so much simpler with a VariantBuilder....
  const auto rec = *it;
  const auto am = rec.allele_mask();
  BOOST_REQUIRE_EQUAL(am.size(), 3u);
  BOOST_CHECK(am[0] == AlleleType::REFERENCE);
  BOOST_CHECK(am[1] == AlleleType::DELETION);
  BOOST_CHECK(am[2] == AlleleType::INSERTION);
}

BOOST_AUTO_TEST_CASE( test_missing_variant_record ) {
  auto header = SingleVariantReader{"testdata/test_variants.vcf"}.header();
  auto builder = VariantBuilder{header};

  auto missing_variant = Variant{};
  auto non_missing_variant = builder.set_chromosome("20").set_alignment_start(5).set_ref_allele("A").build();

  BOOST_CHECK(missing(missing_variant));
  BOOST_CHECK(! missing(non_missing_variant));
}

BOOST_AUTO_TEST_CASE( test_missing_integer_individual_field_values ) {
  auto header = SingleVariantReader{"testdata/test_variants.vcf"}.header();
  auto builder = VariantBuilder{header};
  builder.set_chromosome("20").set_alignment_start(5).set_ref_allele("A");

  // Missing integer field value of length one (will have just a missing value, no vector end)
  auto variant = builder.set_integer_individual_field("VLINT", vector<vector<int32_t>>{{1}, {}, {3}}).build();
  BOOST_CHECK(! missing(variant.integer_individual_field("VLINT")[0]));
  BOOST_CHECK(missing(variant.integer_individual_field("VLINT")[1]));
  BOOST_CHECK(! missing(variant.integer_individual_field("VLINT")[2]));

  // Missing integer field value of length two (will have both a missing and a vector end value)
  variant = builder.set_integer_individual_field("VLINT", vector<vector<int32_t>>{{1, 2}, {}, {3}}).build();
  BOOST_CHECK(! missing(variant.integer_individual_field("VLINT")[0]));
  BOOST_CHECK(missing(variant.integer_individual_field("VLINT")[1]));
  BOOST_CHECK(! missing(variant.integer_individual_field("VLINT")[2]));
}

BOOST_AUTO_TEST_CASE( test_missing_float_individual_field_values ) {
  auto header = SingleVariantReader{"testdata/test_variants.vcf"}.header();
  auto builder = VariantBuilder{header};
  builder.set_chromosome("20").set_alignment_start(5).set_ref_allele("A");

  // Missing float field value of length one (will have just a missing value, no vector end)
  auto variant = builder.set_float_individual_field("VLFLOAT", vector<vector<float>>{{1.0}, {}, {3.0}}).build();
  BOOST_CHECK(! missing(variant.float_individual_field("VLFLOAT")[0]));
  BOOST_CHECK(missing(variant.float_individual_field("VLFLOAT")[1]));
  BOOST_CHECK(! missing(variant.float_individual_field("VLFLOAT")[2]));

  // Missing float field value of length two (will have both a missing and a vector end value)
  variant = builder.set_float_individual_field("VLFLOAT", vector<vector<float>>{{1.0, 2.0}, {}, {3.0}}).build();
  BOOST_CHECK(! missing(variant.float_individual_field("VLFLOAT")[0]));
  BOOST_CHECK(missing(variant.float_individual_field("VLFLOAT")[1]));
  BOOST_CHECK(! missing(variant.float_individual_field("VLFLOAT")[2]));
}

BOOST_AUTO_TEST_CASE( test_missing_string_individual_field_values ) {
  auto header = SingleVariantReader{"testdata/test_variants.vcf"}.header();
  auto builder = VariantBuilder{header};
  builder.set_chromosome("20").set_alignment_start(5).set_ref_allele("A");

  // String field value with empty string
  auto variant = builder.set_string_individual_field("AS", vector<string>{"abc", "", "def"}).build();
  BOOST_CHECK(! missing(variant.string_individual_field("AS")[0]));
  BOOST_CHECK(missing(variant.string_individual_field("AS")[1]));
  BOOST_CHECK(! missing(variant.string_individual_field("AS")[2]));

  // String field value with "."
  variant = builder.set_string_individual_field("AS", vector<string>{"abc", ".", "def"}).build();
  BOOST_CHECK(! missing(variant.string_individual_field("AS")[0]));
  BOOST_CHECK(missing(variant.string_individual_field("AS")[1]));
  BOOST_CHECK(! missing(variant.string_individual_field("AS")[2]));
}

BOOST_AUTO_TEST_CASE( test_field_lengths ) {
  auto header = SingleVariantReader{"testdata/test_variants.vcf"}.header();
  BOOST_CHECK_EQUAL(header.field_length_descriptor("AF", BCF_HL_INFO), static_cast<uint32_t>(BCF_VL_A));
  BOOST_CHECK_EQUAL(header.field_length_descriptor("AF", BCF_HL_FMT), static_cast<uint32_t>(BCF_VL_FIXED));
  BOOST_CHECK_EQUAL(header.field_length_descriptor("PL", BCF_HL_FMT), static_cast<uint32_t>(BCF_VL_G));
  BOOST_CHECK_EQUAL(header.field_length_descriptor("AS", BCF_HL_FMT), static_cast<uint32_t>(BCF_VL_FIXED));
  BOOST_CHECK_EQUAL(header.field_length_descriptor("VLINT", BCF_HL_FMT), static_cast<uint32_t>(BCF_VL_VAR));
  
  BOOST_CHECK_EQUAL(header.field_length("AF", BCF_HL_INFO), 0xfffffu);
  BOOST_CHECK_EQUAL(header.field_length("AF", BCF_HL_FMT), 2u);
  BOOST_CHECK_EQUAL(header.field_length("PL", BCF_HL_FMT), 0xfffffu);
  BOOST_CHECK_EQUAL(header.field_length("AS", BCF_HL_FMT), 1u);
  BOOST_CHECK_EQUAL(header.field_length("VLINT", BCF_HL_FMT), 0xfffffu);
}

