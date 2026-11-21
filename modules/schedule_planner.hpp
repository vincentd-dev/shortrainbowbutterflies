#ifndef PLANNER_H // Prevents multiple inclusion of planner.h
#define PLANNER_H // Marks PLANNER_H as defined

#include <string> // Enables string parameter

// OFFLINE MODE, NOTES & SCHEDULE PLANNER: Manages offline notes, calendar/study schedules, tasks, and deadlines
void runPlannerAndNotesModule(const std::string& username);

#endif // End of PLANNER_H