//
//  main.cpp
//  RunSslServer
//
//  Created by Michael Jones on 15/11/2015.
//  Copyright � 2015 Michael Jones. All rights reserved.
//

#if defined(_WIN32_)
#include "stdafx.h"
#endif

#include "StreamConnection.h"
#include "IoServices.h"
#include "IoLogConsumer.h"
#include <openssl/err.h>
#include <boost/asio.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>
#include <iostream>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <chrono>
#include <thread>

using namespace std; // For atoi.
using namespace std::chrono;
using namespace std::this_thread;
using namespace AsyncIo;

namespace po = boost::program_options;

#if defined(__clang__) || defined(__GNUC__)
int main(int argc, char* argv[]) {
	auto wideargs = vector<wchar_t*>(argc);

	auto wargv = wideargs.data();
	auto wargstrings = vector<wstring>{};

	for (int x = 0; x < argc; x++) {
		string argstring = argv[x];

		wargstrings.push_back(wstring(argstring.begin(), argstring.end()));
	}

	for (int x = 0; x < argc; x++) {
		wideargs[x] = (const_cast<wchar_t*>(wargstrings[x].data()));
	}

#else
int _tmain(int argc, wchar_t* wargv[]) {
#endif
	// insert code here...
	try {
		po::options_description desc("Allowed options");
#if 0
		("cert", po::wvalue<wstring>()->default_value(wstring(L"fd-serv.crt")), "server certificate");
#endif
		desc.add_options()
			("port", po::value<int>()->default_value(23), "server port")
			("cert", po::wvalue<wstring>(), "server certificate")
			("private-key", po::wvalue<wstring>(), "private key")
			("diffie-hellman", po::wvalue<wstring>(), "DH key exchange")
			("password", po::wvalue<wstring>(), "password")
			("verify-client", "verify client")
			("client-file", po::wvalue<wstring>(), "Client verify file");

		po::variables_map vm;
		auto parsed =
			po::wcommand_line_parser(argc, wargv).options(desc).run();

		po::store(parsed, vm);

		//po::store(po::parse_command_line(argc, argv, desc), vm);
		// po::notify(vm);

		if (vm.count("help")) {
			cout << desc << "\n";
			return 1;
		}

		wstring password{};
		wstring cert{ L"fd-serv.crt" };
		wstring privateKey{ L"fd-rsa-priv.key" };
		wstring diffieHellman{ L"dh1024.pem" };
		// "client.pem" did not work. The root certificate seems to be needed.
		wstring clientVerifyFile{ L"CARoot.pem" };

		if (vm.count("cert") > 0)
		{
			cert = vm["cert"].as<wstring>();
		}

		if (vm.count("private-key"))
		{
			privateKey = vm["private-key"].as<wstring>();
		}

		if (vm.count("diffie-hellman"))
		{
			diffieHellman = vm["diffie-hellman"].as<wstring>();
		}

		if (vm.count("client-file")) {
			clientVerifyFile = vm["client-file"].as<wstring>();
		}

		bool verifyClient{};
		if (vm.count("verify-client")) {
			verifyClient = true;
		}

		auto port = vm["port"].as<int>();

		if (vm.count("password")) {
			password = vm["password"].as<wstring>();
		}
		else {
			cout << "password was not set.\n";
			return -1;
		}

		// auto cert = vm["cert"].as<wstring>();

		cout << "Port: " << port << endl;
		boost::asio::io_service io_service;
		//server s(io_service, atoi(argv[1]));

		auto ReadStreamCallback = [](std::shared_ptr<StreamConnection>, int bytesAvailable) {
			cout << "got bytes: " << bytesAvailable << endl;
		};

		auto AcceptStreamCallback = [](std::shared_ptr<StreamConnection> acceptedConn) {

		};

		auto exceptionReporter = [](const std::string& msg, const std::exception&) {

		};

		auto logFn = [](IoLog&& ioLog) {
			wcout << move(ioLog);
		};

		IoLogConsumer::AttachLogger(move(logFn));

		IoServices serviceInstance(move(exceptionReporter));
		serviceInstance.Start();

		std::cout << "Hello, World!\n";

		auto getPwd = [&password]() -> std::string {
			return std::string(password.begin(), password.end());
		};

		SecurityOptions options{};

		options.CertificateFilename = std::string(cert.begin(), cert.end());
		options.PrivKeyFilename = std::string(privateKey.begin(), privateKey.end());
		options.DHExchangeFilename = std::string(diffieHellman.begin(), diffieHellman.end());
		options.GetPasswordCallback = getPwd;
		options.VerifyClient = verifyClient;
		options.ClientVerifyFile = std::string(clientVerifyFile.begin(), clientVerifyFile.end());

		serviceInstance.AddTcpServer(port, move(AcceptStreamCallback), move(ReadStreamCallback));
		serviceInstance.StartTcpServer(port, move(options));

		while (true)
		{
			sleep_for(seconds(2));
		}
	}
	catch (std::exception& e)
	{
		SSL_load_error_strings();
		unsigned long n = ERR_get_error();
		char buf[1024];
		printf("%s\n", ERR_error_string(n, buf));
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}

