#include <gtest/gtest.h>
#include "../include/track.h"

/* 
IMPORTANT: need to be updated if track.cpp adds more detail, 
           as the following information won't be up-to-date, which would give
           the wrong results. The functionality is already checked correct
*/


class TestTrack : public ::testing::Test{
    protected:
        Track track = Track();
};

// Check Constructor is correct
TEST_F(TestTrack, CheckConstructor){
    EXPECT_DOUBLE_EQ(track.at(0)->get_length(), 458.0);
    EXPECT_DOUBLE_EQ(track.at(1)->get_length(), 167.0);
    EXPECT_DOUBLE_EQ(track.at(2)->get_length(), 262.0);
    EXPECT_DOUBLE_EQ(track.at(3)->get_length(), 0.0);
    EXPECT_DOUBLE_EQ(track.at(6)->get_length(), 667.0);
    EXPECT_DOUBLE_EQ(track.at(8)->get_length(), 476.0);
    EXPECT_DOUBLE_EQ(track.at(10)->get_length(), 150.0);
    EXPECT_DOUBLE_EQ(track.at(13)->get_length(), 0.0);
    EXPECT_DOUBLE_EQ(track.at(15)->get_length(), 0.0);
    EXPECT_EQ(track.size(), 16);
}

// Check if moving the index is correctly moved around
// Edges cases included: moving before 0 gives the last index,
//                       moving after last gives the first index
TEST_F(TestTrack, CheckCorrectIndexChanage){
    EXPECT_EQ(track.get_index(), 0);
    track.incre();      // Move to segment 1
    EXPECT_EQ(track.get_index(), 1);
    track.decre();      // Move back to start
    EXPECT_EQ(track.get_index(), 0);
    track.decre();      // Move to the end 
    EXPECT_EQ(track.get_index(), 15);
    track.incre();      // Move back to the start
    EXPECT_EQ(track.get_index(), 0);
    track.decre();      // Move to second last
    track.decre();
    EXPECT_EQ(track.get_index(), 14);
    track.reset();      // Reset to start
    EXPECT_EQ(track.get_index(), 0);
}

// Check if segments are outputted correctly
// Edges cases included: same as above
TEST_F(TestTrack, CheckCorrectSegmentPointer){
    EXPECT_DOUBLE_EQ(track.end()->get_length(), 0.0);
    
    EXPECT_DOUBLE_EQ(track.current()->get_length(), 458.0);
    EXPECT_DOUBLE_EQ(track.prev()->get_length(), 0.0);
    EXPECT_DOUBLE_EQ(track.prev(0)->get_length(), 0.0);
    EXPECT_DOUBLE_EQ(track.prev(3)->get_length(), 262.0);
    EXPECT_DOUBLE_EQ(track.next()->get_length(), 167.0);
    EXPECT_DOUBLE_EQ(track.next(15)->get_length(), 458.0);
    EXPECT_DOUBLE_EQ(track.next(1)->get_length(), 262.0);
    track.incre();      // Move to segment 1
    EXPECT_DOUBLE_EQ(track.current()->get_length(), 167.0);
    EXPECT_DOUBLE_EQ(track.prev()->get_length(), 458.0);
    EXPECT_DOUBLE_EQ(track.next()->get_length(), 262.0);
    track.decre();      // Move to last segment
    track.decre();
    EXPECT_DOUBLE_EQ(track.current()->get_length(), 0.0);
    EXPECT_DOUBLE_EQ(track.prev()->get_length(), 286.0);
    EXPECT_DOUBLE_EQ(track.next()->get_length(), 458.0);

    EXPECT_DOUBLE_EQ(track.begin()->get_length(), 458.0); 
}

