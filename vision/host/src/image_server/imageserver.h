/*
 * imageserver.h
 *
 *  Created on: Apr 10, 2023
 *      Author: overman
 */

#pragma once


#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>

using namespace std;
using namespace cv;
using boost::asio::ip::tcp;

std::vector<std::string> split(const std::string &str, const std::string &delim) ;

std::mutex g_mutexImageServers;

map<thread::id,bool> mapImageServers;

bool CheckShutdown(std::thread::id id) ;

thread *startImageServer(int cameraId = 0, int port = 8080) ;
