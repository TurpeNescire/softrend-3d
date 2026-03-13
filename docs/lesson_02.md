---
layout: lesson
title: "Lesson 02: Drawing to the screen"
lesson_label: "Lesson 02"
prev_lesson: /lesson_01
next_lesson: /lesson_03
---

# Work in progress

## time paced loops
`#define avg_frame_time_ms (1000.0f / fps)` becomes a float, but 

    int64_t now = fenster_time();
    ...
    int64_t elapsed = fenster_time() - now;
    int64_t remaining = (int64_t)avg_frame_time_ms - elapsed;
    if (remaining > 0) fenster_sleep((int64_t)remaining);

`fenster_time()` returns time since the [epoch](https://en.wikipedia.org/wiki/unix_time) in milliseconds, which will pass 2 trillion ms some time in 2033, and uint64_t is an 8 byte value that can hold numbers up to 18 quintillion or so, which will take somewhere around 584 billion years before an overflow will become an issue. using an unsigned 4 byte value (`uint32_t`), overflow would only become a factor in about 80 years, so i guess the fenster author was being cautious! we do have to deal with `fenster_sleep()` only taking an integer value, which will truncate our remaining time in milliseconds (12.842 
