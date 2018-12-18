#include <gtest/gtest.h>
#include "list.h"
#include <random>
#include <list>

TEST(ListTest, SimpleTest) {
    List<int> list;
    list.PushBack(1);
    list.PushBack(2);
    list.PushFront(3);
    ASSERT_EQ(list.Front(), 3);
    ASSERT_EQ(list.Back(), 2);
    list.PopBack();
    ASSERT_EQ(list.Front(), 3);
    ASSERT_EQ(list.Back(), 1);
    list.PopFront();
    ASSERT_EQ(list.Front(), 1);
    ASSERT_EQ(list.Back(), 1);
}

TEST(ListTest, Iterators) {
    List<int> list;
    for (int i = 0; i < 10; ++i) {
        list.PushBack(i);
    }
    List<int>::Iterator iterator = list.begin();
    while (*iterator != 5) {
        ++iterator;
    }
    iterator = list.InsertBefore(iterator, -1);
    ASSERT_EQ(*iterator, -1);
    for (int i = 0; i < 5; ++i) {
        list.PopFront();
        list.PopBack();
    }
    ASSERT_EQ(*iterator, -1);
    ASSERT_EQ(list.Size(), 1u);
    ASSERT_EQ(list.Front(), -1);
    ASSERT_EQ(list.Back(), -1);
}

TEST(ListTest, Stress) {
    std::mt19937 generator(123);
    std::list<int> check;
    List<int> list;
    list.PushBack(0);
    check.push_back(0);
    int n = 10000;
    for (int i = 0; i < n; ++i) {
        switch (generator() % 2) {
            case 0: {
                int value = generator();
                list.PushBack(value);
                check.push_back(value);
                break;
            }
            case 1: {
                int value = generator();
                list.PushFront(value);
                check.push_front(value);
                break;
            }
        }
        ASSERT_EQ(list.Back(), check.back());
        ASSERT_EQ(list.Front(), check.front());
    }
    for (int i = 0; i < n; ++i) {
        switch (generator() % 2) {
            case 0: {
                list.PopBack();
                check.pop_back();
                break;
            }
            case 1: {
                list.PopFront();
                check.pop_front();
                break;
            }
        }
        ASSERT_EQ(list.Back(), check.back());
        ASSERT_EQ(list.Front(), check.front());
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}