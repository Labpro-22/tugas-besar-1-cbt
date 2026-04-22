#include "../../../include/models/GameManager/LogEntry.hpp"


LogEntry::LogEntry(int turn, string username, string actionType, string detail) : turn(turn), username(username), actionType(actionType), detail(detail) {}