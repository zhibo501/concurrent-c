#include<iostream>
using namespace std; 
#include<gtest/gtest.h>

#include "rcu.h"

TEST(AdderTest, IsAdderOK)
{
    ASSERT_TRUE(1 + 2 == 3) << "adder(1, 2)=3";  //ASSERT_TRUE期待结果是true,operator<<输出一些自定义的信息
}
