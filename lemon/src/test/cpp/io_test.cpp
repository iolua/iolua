#include <lemon/io/io.hpp>
#include <lemon/log/log.hpp>
#include <lemon/test/test.hpp>

using namespace lemon::io;

auto &logger = lemon::log::get("test");


test_(socket)
{
	io_service ioservice;

	auto addrinfo = lemon::io::getaddrinfo("", "1812", AF_INET, SOCK_STREAM, AI_PASSIVE)[0];

	io_socket_acceptor acceptor(ioservice, addrinfo.af(), addrinfo.type(), addrinfo.protocol());

	acceptor.bind(addrinfo.addr());

	acceptor.listen(SOMAXCONN);

	io_stream_socket * stream;

	char recvbuff[1024];

	bool exit = false;

	acceptor.accept([&](std::unique_ptr<io_socket> & socket, address && addr, const std::error_code & ec) {
		if (ec)
		{
			lemonE(logger, "accept client error :%s", ec.message().c_str());
		}
		else
		{
			lemonI(logger, "accept client(%s:%d) success ",addr.host().c_str(),addr.service());

			stream = new io_stream_socket(socket);

			stream->recv(buff(recvbuff), 0, [&](size_t trans, const std::error_code &ec) {
				if (ec)
				{
					lemonE(logger, "recv message from client error :%s", ec.message().c_str());
				}
				else
				{
					lemonI(logger, "recv message from client success(%s)", std::string(recvbuff,recvbuff+trans).c_str());
				}

				exit = true;
			});
		}

	});
	
	
	io_stream_socket client(ioservice, AF_INET, SOCK_STREAM, IPPROTO_TCP);

	client.connect(addrinfo.addr(),[&](const std::error_code& ec){
		if(ec)
		{
			lemonE(logger, "connect service error :%s",ec.message().c_str());
		}
		else
		{
			lemonI(logger, "connect success");

			client.send(cbuff("hello world"), 0, [](size_t trans, const std::error_code &ec) {
				if (ec)
				{
					lemonE(logger, "send message to service error :%s", ec.message().c_str());
				}
				else
				{
					lemonI(logger, "send message to service success(%d)",trans);
				}
			});
		}
	});

	while (!exit) 
	{
		ioservice.run_one(std::chrono::seconds(1));
	}
}