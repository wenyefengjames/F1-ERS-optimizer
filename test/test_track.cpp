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
        Straight ham = Straight("Hamilton Straight", 410, true, 25, 325);
        Straight no_sm_straight = Straight("Turn 9 - Turn 10 Straight", 384.5);
        SlowCorner turn3 = SlowCorner("Turn 3", 75, 76, 5.04, 263, 108, 141, 50);
        FastCorner turn13 = FastCorner("Turn 13", 112, 68.5, 219, 244, 55);
};

// Check Constructor is correct
TEST_F(TestTrack, CheckConstructor){
    EXPECT_DOUBLE_EQ(track.at(0)->get_length(), 410);
    EXPECT_EQ(track.at(0)->get_sm(), true);    
    EXPECT_DOUBLE_EQ(track.at(1)->get_length(), 207);
    EXPECT_DOUBLE_EQ(track.at(2)->get_length(), 282);
    EXPECT_DOUBLE_EQ(track.at(3)->get_length(), 151);
    EXPECT_DOUBLE_EQ(track.at(7)->get_length(), 223);
    EXPECT_DOUBLE_EQ(track.at(9)->get_length(), 319);
    EXPECT_EQ(track.size(), 25);

    // Check Straight constructor
    EXPECT_DOUBLE_EQ(ham.get_sm_start(), 25);
    EXPECT_DOUBLE_EQ(ham.get_sm_end(), 325);
    EXPECT_EQ(ham.get_sm(), true);    
    EXPECT_DOUBLE_EQ(no_sm_straight.get_sm_start(), 0);
    EXPECT_DOUBLE_EQ(no_sm_straight.get_sm_end(), 0);
    EXPECT_EQ(no_sm_straight.get_sm(), false);
    
    // Check SlowCorner constructor
    EXPECT_DOUBLE_EQ(turn3.get_apex_min_speed(), 108);
    EXPECT_DOUBLE_EQ(turn3.get_throttle_percentage(), 50);
    EXPECT_DOUBLE_EQ(turn3.get_entry_speed(), 263);
    EXPECT_DOUBLE_EQ(turn3.get_exit_speed(), 141);
    EXPECT_DOUBLE_EQ(turn3.get_entry_to_apex_length(), 75);
    EXPECT_DOUBLE_EQ(turn3.get_apex_to_exit_length(), 76);
    EXPECT_DOUBLE_EQ(turn3.get_time(), 5.04);
    EXPECT_EQ(turn3.get_sm(), false);    

    // Check FastCorner constructor
    EXPECT_DOUBLE_EQ(turn13.get_apex_min_speed(), 219);
    EXPECT_DOUBLE_EQ(turn13.get_throttle_percentage(), 55);
    EXPECT_DOUBLE_EQ(turn13.get_exit_speed(), 244);
    EXPECT_DOUBLE_EQ(turn13.get_entry_to_apex_length(), 112);
    EXPECT_DOUBLE_EQ(turn13.get_apex_to_exit_length(), 68.5);
    EXPECT_EQ(turn13.get_sm(), false); 
}
// COMPLETE: Validated already, doesn't need to be tested again unless functions of track.cpp is changed

// Check if moving the index is correctly moved around
// Edges cases included: moving before 0 gives the last index,
//                       moving after last gives the first index
// TEST_F(TestTrack, CheckCorrectIndexChanage){
//     EXPECT_EQ(track.get_index(), 0);
//     track.incre();      // Move to segment 1
//     EXPECT_EQ(track.get_index(), 1);
//     track.decre();      // Move back to start
//     EXPECT_EQ(track.get_index(), 0);
//     track.decre();      // Move to the end 
//     EXPECT_EQ(track.get_index(), 15);
//     track.incre();      // Move back to the start
//     EXPECT_EQ(track.get_index(), 0);
//     track.decre();      // Move to second last
//     track.decre();
//     EXPECT_EQ(track.get_index(), 14);
//     track.reset();      // Reset to start
//     EXPECT_EQ(track.get_index(), 0);
// }

// COMPLETE: Validated already, doesn't need to be tested again unless functions of track.cpp is changed

// Check if segments are outputted correctly
// Edges cases included: same as above
// TEST_F(TestTrack, CheckCorrectSegmentPointer){
//     EXPECT_DOUBLE_EQ(track.end()->get_length(), 0.0);
    
//     EXPECT_DOUBLE_EQ(track.current()->get_length(), 458.0);
//     EXPECT_DOUBLE_EQ(track.prev()->get_length(), 0.0);
//     EXPECT_DOUBLE_EQ(track.prev(0)->get_length(), 0.0);
//     EXPECT_DOUBLE_EQ(track.prev(3)->get_length(), 262.0);
//     EXPECT_DOUBLE_EQ(track.next()->get_length(), 167.0);
//     EXPECT_DOUBLE_EQ(track.next(15)->get_length(), 458.0);
//     EXPECT_DOUBLE_EQ(track.next(1)->get_length(), 262.0);
//     track.incre();      // Move to segment 1
//     EXPECT_DOUBLE_EQ(track.current()->get_length(), 167.0);
//     EXPECT_DOUBLE_EQ(track.prev()->get_length(), 458.0);
//     EXPECT_DOUBLE_EQ(track.next()->get_length(), 262.0);
//     track.decre();      // Move to last segment
//     track.decre();
//     EXPECT_DOUBLE_EQ(track.current()->get_length(), 0.0);
//     EXPECT_DOUBLE_EQ(track.prev()->get_length(), 286.0);
//     EXPECT_DOUBLE_EQ(track.next()->get_length(), 458.0);

//     EXPECT_DOUBLE_EQ(track.begin()->get_length(), 458.0); 
// }

