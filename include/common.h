#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <deque>
#include <map>
#include <list>
#include <set>
#include <queue>
#include <functional>
#include <fstream>
#include <sstream>
#include <iterator>
#include <memory>
#include <typeinfo>

#ifdef __linux__
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif
