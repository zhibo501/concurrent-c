#include <iostream>
using namespace std; 
#include <gtest/gtest.h>
// #include "mockcpp/mockcpp.hpp"

#include <stdint.h>
#include "rcu.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void share_debug_show(void* self);

int32_t g_usr_data_default = 0;
void* rcu_test_clone(const void* usr_data)
{
	int32_t* p = (int32_t*)malloc(sizeof(int32_t));
	*p = *(int32_t*)usr_data;
	return p;
}
void rcu_test_free(void* usr_data)
{
	if (usr_data != (void*)&g_usr_data_default) {
		free(usr_data);
	}
}
int rcu_test_read_in_read_task(const void* usr_data, void* option)
{
	int32_t left = *(int32_t*)usr_data;
	int32_t right = *(int32_t*)option;
	if (left != right) {
		// printf("====== rcu_test_read_in_read_task read != want: %d != %d\n", left, right);
		return -1;
	}
	return 0;
}
int rcu_test_read_in_write_task(const void* usr_data, void* option)
{
	int32_t left = *(int32_t*)usr_data;
	int32_t right = *(int32_t*)option;
	if (left != right) {
		printf("====== rcu_test_read_in_write_task read != want: %d != %d\n", left, right);
		return -1;
	}
	return 0;
}
int rcu_test_write(void* usr_data, void* option)
{
	*(int32_t*)usr_data = *(int32_t*)option;
	return 0;
}

#define RCU_TEST_LOOPS     (1000000)
int32_t g_rcu_test_var;

void* rcu_test_write_thread(void* sd)
{
	while (1) {
		for (int32_t i = 0; i < RCU_TEST_LOOPS; i++) {
			int32_t tmp = g_rcu_test_var;
			share_write(sd, rcu_test_write, &tmp);
			if (share_read(sd, rcu_test_read_in_write_task, &tmp)) {
				share_debug_show(sd);
			}
		}
		usleep(1);
	}
	return 0;
}
void* rcu_test_read_thread(void* sd)
{
	while (1) {
		for (int32_t i = 0; i < RCU_TEST_LOOPS; i++) {
			int32_t tmp = g_rcu_test_var;
			if (share_read(sd, rcu_test_read_in_read_task, &tmp)) {
				share_debug_show(sd);
			}
		}
		usleep(1);
	}
	return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */


class RcuParallelFixture : public testing::Test
{
protected:
	static void SetUpTestCase() {}
	static void TearDownTestCase() {}
	void SetUp() {}
	void TearDown() {
		// GlobalMockObject::reset();
	}
};

TEST_F(RcuParallelFixture, rcu_parallel_test)
{
	void* sd = new_share_data(8, rcu_test_clone, rcu_test_free, &g_usr_data_default);
	printf("test case -- new_share_data %p (8, %p, %p, %p)\n",
		sd, rcu_test_clone, rcu_test_free, &g_usr_data_default);

	/* wirte task 1 */
	pthread_t tid1;
	pthread_create(&tid1, NULL, rcu_test_write_thread, sd);
	/* wirte task 2 */
	pthread_t tid2;
	pthread_create(&tid2, NULL, rcu_test_write_thread, sd);

	/* read task 1 */
	pthread_t tid3;
	pthread_create(&tid3, NULL, rcu_test_read_thread, sd);
	/* read task 2 */
	pthread_t tid4;
	pthread_create(&tid4, NULL, rcu_test_read_thread, sd);
	/* read task 3 */
	pthread_t tid5;
	pthread_create(&tid5, NULL, rcu_test_read_thread, sd);
	/* read task 4 */
	pthread_t tid6;
	pthread_create(&tid6, NULL, rcu_test_read_thread, sd);
	/* read task 5 */
	pthread_t tid7;
	pthread_create(&tid7, NULL, rcu_test_read_thread, sd);

	/* read task in test case */
	for (int32_t i = 100; i < 110; i++) {
		g_rcu_test_var = i;
		usleep(100);
		for (int32_t j = 0; j < RCU_TEST_LOOPS; j++) {
			share_read(sd, rcu_test_read_in_read_task, &g_rcu_test_var);
		}
	}
}

TEST(AdderTest, IsAdderOK)
{
    ASSERT_TRUE(1 + 2 == 3) << "adder(1, 2)=3";  //ASSERT_TRUE期待结果是true,operator<<输出一些自定义的信息
}
