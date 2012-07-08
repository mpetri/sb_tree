#include "gtest/gtest.h"

#include "critbit_tree.h"

TEST(critbit , CRITBIT_ISLEAF)
{
    uint64_t leaf1 = 0xF001231231231234;
    uint64_t leaf2 = 0x8000000123123213;
    uint64_t leaf3 = 0x8123123213123113;
    uint64_t leaf4 = 0xE123123213123113;

    uint64_t nonleaf1 = 0x7123123213123113;
    uint64_t nonleaf2 = 0x0012323213123113;
    uint64_t nonleaf3 = 0x0000023213123113;
    uint64_t nonleaf4 = 0x0000000000000000;

    EXPECT_EQ(CRITBIT_ISLEAF(leaf1) , 1);
    EXPECT_EQ(CRITBIT_ISLEAF(leaf2) , 1);
    EXPECT_EQ(CRITBIT_ISLEAF(leaf3) , 1);
    EXPECT_EQ(CRITBIT_ISLEAF(leaf4) , 1);

    EXPECT_EQ(CRITBIT_ISLEAF(nonleaf1) , 0);
    EXPECT_EQ(CRITBIT_ISLEAF(nonleaf2) , 0);
    EXPECT_EQ(CRITBIT_ISLEAF(nonleaf3) , 0);
    EXPECT_EQ(CRITBIT_ISLEAF(nonleaf4) , 0);
}

TEST(critbit , CRITBIT_SETSUFFIX)
{
    uint64_t suffix1 = 0x0000000000000001;
    uint64_t suffix2 = 0x0000000123123213;
    uint64_t suffix3 = 0x0000000213123113;
    uint64_t suffix4 = 0x0F12323213123113;

    uint64_t setsuffix1 = 0x8000000000000001;
    uint64_t setsuffix2 = 0x8000000123123213;
    uint64_t setsuffix3 = 0x8000000213123113;
    uint64_t setsuffix4 = 0x8F12323213123113;

    EXPECT_EQ((uint64_t)CRITBIT_SETSUFFIX(suffix1) , setsuffix1);
    EXPECT_EQ((uint64_t)CRITBIT_SETSUFFIX(suffix2) , setsuffix2);
    EXPECT_EQ((uint64_t)CRITBIT_SETSUFFIX(suffix3) , setsuffix3);
    EXPECT_EQ((uint64_t)CRITBIT_SETSUFFIX(suffix4) , setsuffix4);
}

TEST(critbit , CRITBIT_GETSUFFIX)
{
    uint64_t suffix1 = 0x0000000000000001;
    uint64_t suffix2 = 0x0000000123123213;
    uint64_t suffix3 = 0x0000000213123113;
    uint64_t suffix4 = 0x0F12323213123113;

    uint64_t setsuffix1 = 0x8000000000000001;
    uint64_t setsuffix2 = 0x8000000123123213;
    uint64_t setsuffix3 = 0x8000000213123113;
    uint64_t setsuffix4 = 0x8F12323213123113;

    EXPECT_EQ(CRITBIT_GETSUFFIX(setsuffix1) , suffix1);
    EXPECT_EQ(CRITBIT_GETSUFFIX(setsuffix2) , suffix2);
    EXPECT_EQ(CRITBIT_GETSUFFIX(setsuffix3) , suffix3);
    EXPECT_EQ(CRITBIT_GETSUFFIX(setsuffix4) , suffix4);
}

TEST(critbit , CRITBIT_GETBITPOS)
{
    uint64_t critbit_pos1 = 4005;
    uint64_t critbit_pos2 = 7207;
    uint64_t critbit_pos3 = 32000;
    uint64_t critbit_pos4 = 1;

    uint64_t critbit_byte_pos1 = 500;
    uint64_t critbit_byte_pos2 = 900;
    uint64_t critbit_byte_pos3 = 4000;
    uint64_t critbit_byte_pos4 = 0;

    uint64_t critbit_bit_pos1 = 5;
    uint64_t critbit_bit_pos2 = 7;
    uint64_t critbit_bit_pos3 = 0;
    uint64_t critbit_bit_pos4 = 1;

    EXPECT_EQ(CRITBIT_GETBITPOS(critbit_pos1) , critbit_bit_pos1);
    EXPECT_EQ(CRITBIT_GETBITPOS(critbit_pos2) , critbit_bit_pos2);
    EXPECT_EQ(CRITBIT_GETBITPOS(critbit_pos3) , critbit_bit_pos3);
    EXPECT_EQ(CRITBIT_GETBITPOS(critbit_pos4) , critbit_bit_pos4);
}

TEST(critbit , CRITBIT_GETBYTEPOS)
{
    uint64_t critbit_pos1 = 4005;
    uint64_t critbit_pos2 = 7207;
    uint64_t critbit_pos3 = 32000;
    uint64_t critbit_pos4 = 1;

    uint64_t critbit_byte_pos1 = 500;
    uint64_t critbit_byte_pos2 = 900;
    uint64_t critbit_byte_pos3 = 4000;
    uint64_t critbit_byte_pos4 = 0;

    uint64_t critbit_bit_pos1 = 5;
    uint64_t critbit_bit_pos2 = 7;
    uint64_t critbit_bit_pos3 = 0;
    uint64_t critbit_bit_pos4 = 1;

    EXPECT_EQ(CRITBIT_GETBYTEPOS(critbit_pos1) , critbit_byte_pos1);
    EXPECT_EQ(CRITBIT_GETBYTEPOS(critbit_pos2) , critbit_byte_pos2);
    EXPECT_EQ(CRITBIT_GETBYTEPOS(critbit_pos3) , critbit_byte_pos3);
    EXPECT_EQ(CRITBIT_GETBYTEPOS(critbit_pos4) , critbit_byte_pos4);
}

TEST(critbit , CRITBIT_GETCRITBITPOS)
{
    uint8_t sym1 = 'a'; /* 97  : 01100001 */
    uint8_t sym2 = 't'; /* 116 : 01110100 */
    uint8_t sym3 = 'z'; /* 122 : 01111010 */
    uint8_t sym4 = ','; /* 44  : 00101100 */
    uint8_t sym5 = 212; /* 212 : 11010100 */
    uint8_t sym6 = 'u'; /* 117 : 01110101 */

    uint64_t critbit_bit_pos12 = 3;
    uint64_t critbit_bit_pos13 = 3;
    uint64_t critbit_bit_pos14 = 1;
    uint64_t critbit_bit_pos23 = 4;
    uint64_t critbit_bit_pos24 = 1;
    uint64_t critbit_bit_pos34 = 1;

    uint64_t critbit_bit_pos5 = 0;
    uint64_t critbit_bit_pos26 = 7;


    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym1,sym2) , critbit_bit_pos12);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym2,sym1) , critbit_bit_pos12);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym1,sym3) , critbit_bit_pos13);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym1,sym4) , critbit_bit_pos14);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym2,sym3) , critbit_bit_pos23);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym2,sym4) , critbit_bit_pos24);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym3,sym4) , critbit_bit_pos34);

    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym5,sym1) , critbit_bit_pos5);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym5,sym2) , critbit_bit_pos5);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym5,sym3) , critbit_bit_pos5);
    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym5,sym4) , critbit_bit_pos5);

    EXPECT_EQ(CRITBIT_GETCRITBITPOS(sym2,sym6) , critbit_bit_pos26);

}



TEST(critbit , CRITBIT_GETDIRECTION)
{
    uint8_t sym1 = 'a'; /* 97  : 01100001 */
    uint8_t sym2 = 't'; /* 116 : 01110100 */
    uint8_t sym3 = 'z'; /* 122 : 01111010 */
    uint8_t sym4 = ','; /* 44  : 00101100 */
    uint8_t sym5 = 212; /* 212 : 11010100 */
    uint8_t sym6 = 'u'; /* 117 : 01110101 */

    uint64_t critbit_bit_pos1_1 = 0;
    uint64_t critbit_bit_pos1_2 = 1;
    uint64_t critbit_bit_pos1_3 = 1;
    uint64_t critbit_bit_pos1_4 = 0;
    uint64_t critbit_bit_pos1_5 = 0;
    uint64_t critbit_bit_pos1_6 = 0;
    uint64_t critbit_bit_pos1_7 = 0;
    uint64_t critbit_bit_pos1_8 = 1;

    EXPECT_EQ(CRITBIT_GETDIRECTION(sym1,0) , critbit_bit_pos1_1);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym1,1) , critbit_bit_pos1_2);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym1,2) , critbit_bit_pos1_3);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym1,3) , critbit_bit_pos1_4);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym1,4) , critbit_bit_pos1_5);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym1,5) , critbit_bit_pos1_6);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym1,6) , critbit_bit_pos1_7);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym1,7) , critbit_bit_pos1_8);

    EXPECT_EQ(CRITBIT_GETDIRECTION(sym5,0) , 1);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym5,1) , 1);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym5,2) , 0);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym5,3) , 1);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym5,4) , 0);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym5,5) , 1);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym5,6) , 0);
    EXPECT_EQ(CRITBIT_GETDIRECTION(sym5,7) , 0);

}

TEST(critbit , insert_suffix)
{
    critbit_tree_t* cbt = critbit_create();

    const char* T = "mississippi$";
    size_t n = strlen(T);

    critbit_insert_suffix(cbt,(const uint8_t*)T,n,5);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,7);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,6);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,2);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,8);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,0);

    critbit_free(cbt);
}

TEST(critbit , delete_suffix)
{
    critbit_tree_t* cbt = critbit_create();

    const char* T = "mississippi$";
    size_t n = strlen(T);

    critbit_insert_suffix(cbt,(const uint8_t*)T,n,5);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,7);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,6);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,2);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,8);
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,0);

    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,0) , 0);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,1) , 1);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,2) , 0);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,3) , 1);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,4) , 1);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,5) , 0);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,6) , 0);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,7) , 0);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,8) , 0);
    EXPECT_EQ(critbit_delete_suffix(cbt,(const uint8_t*)T,n,9) , 1);

    critbit_free(cbt);
}

TEST(critbit , contains)
{
    critbit_tree_t* cbt = critbit_create();

    const char* T = "mississippi$";
    size_t n = strlen(T);

    critbit_insert_suffix(cbt,(const uint8_t*)T,n,5); /* insert ssippi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,7); /* insert ippi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,6); /* insert sippi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,2); /* insert ssissippi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,8); /* insert ppi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,0); /* insert mississippi$ */

    EXPECT_EQ(critbit_contains(cbt,(const uint8_t*)T,n, (const uint8_t*)"s",1) , 1);
    EXPECT_EQ(critbit_contains(cbt,(const uint8_t*)T,n, (const uint8_t*)"ss",2) , 1);
    EXPECT_EQ(critbit_contains(cbt,(const uint8_t*)T,n, (const uint8_t*)"iss",3) , 0);
    EXPECT_EQ(critbit_contains(cbt,(const uint8_t*)T,n, (const uint8_t*)"i$",2) , 0);
    EXPECT_EQ(critbit_contains(cbt,(const uint8_t*)T,n, (const uint8_t*)"ippi$",5) , 1);
    EXPECT_EQ(critbit_contains(cbt,(const uint8_t*)T,n, (const uint8_t*)"mississippi$",12) , 1);
    EXPECT_EQ(critbit_contains(cbt,(const uint8_t*)T,n, (const uint8_t*)"x",1) , 0);
    EXPECT_EQ(critbit_contains(cbt,(const uint8_t*)T,n, (const uint8_t*)"sleep",5) , 0);

    critbit_free(cbt);
}

TEST(critbit , suffixes)
{
    critbit_tree_t* cbt = critbit_create();

    const char* T = "mississippi$";
    size_t n = strlen(T);

    critbit_insert_suffix(cbt,(const uint8_t*)T,n,5); /* insert ssippi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,7); /* insert ippi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,6); /* insert sippi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,2); /* insert ssissippi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,8); /* insert ppi$ */
    critbit_insert_suffix(cbt,(const uint8_t*)T,n,0); /* insert mississippi$ */

    uint64_t* results;
    EXPECT_EQ(critbit_suffixes(cbt,(const uint8_t*)T,n, (const uint8_t*)"s",1,&results) , 3);
    EXPECT_TRUE(results[0] == 2 && results[1] == 5 && results[2] == 6);
    free(results);

    EXPECT_EQ(critbit_suffixes(cbt,(const uint8_t*)T,n, (const uint8_t*)"ss",2,&results) , 2);
    EXPECT_TRUE(results[0] == 2 && results[1] == 5);
    free(results);

    EXPECT_EQ(critbit_suffixes(cbt,(const uint8_t*)T,n, (const uint8_t*)"ippi$",5,&results) , 1);
    EXPECT_EQ(results[0] , 7);
    free(results);

    EXPECT_EQ(critbit_suffixes(cbt,(const uint8_t*)T,n, (const uint8_t*)"mississippi$",12,&results) , 1);
    EXPECT_EQ(results[0] , 0);
    free(results);

    EXPECT_EQ(critbit_suffixes(cbt,(const uint8_t*)T,n, (const uint8_t*)"iss",3,&results) , 0);
    EXPECT_EQ(critbit_suffixes(cbt,(const uint8_t*)T,n, (const uint8_t*)"i$",2,&results) , 0);
    EXPECT_EQ(critbit_suffixes(cbt,(const uint8_t*)T,n, (const uint8_t*)"x",1,&results) , 0);
    EXPECT_EQ(critbit_suffixes(cbt,(const uint8_t*)T,n, (const uint8_t*)"sleep",5,&results) , 0);

    critbit_free(cbt);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

