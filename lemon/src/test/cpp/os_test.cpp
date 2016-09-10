#include <iostream>
#include <lemon/os/os.hpp>
#include <lemon/test/test.hpp>



using namespace lemon::os;

test_(getenv) {

    auto path = lemon::os::getenv("GSMAKE_HOME");

    test_assert(std::get<1>(path));

    lemonI(lemon::log::get("test"),"%s",std::get<0>(path).c_str());
}

test_(lookup) {
#ifdef WIN32
    auto path = lookup("notepad");
#else
    auto path = lookup("ls");
#endif

    test_assert(std::get<1>(path));

    lemonI(lemon::log::get("test"),"%s",std::get<0>(path).c_str());

}

void out_print(exec & c)
{

	static char recv_buff[1024];

	c.out().read(lemon::io::buff(recv_buff),[&](size_t trans, const std::error_code &err){

		if(!err)
		{
			lemonI(lemon::log::get("test"), "%s", std::string(recv_buff, recv_buff + trans).c_str());

			out_print(c);

			return;
		}

		lemonE(lemon::log::get("test"), "%s", err.message().c_str());
	});
}

void err_print(exec & c)
{

	static char recv_buff[1024];

	c.err().read(lemon::io::buff(recv_buff), [&](size_t trans, const std::error_code &err) {

		if (!err)
		{
			lemonI(lemon::log::get("test"), "%s", std::string(recv_buff, recv_buff + trans).c_str());

			err_print(c);

			return;
		}

		lemonE(lemon::log::get("test"), "%s", err.message().c_str());
	});
}


test_(command) {
	exec c("netstat", exec_options((int)exec_options::pipe_in | (int)exec_options::pipe_out)) ;

	out_print(c);

	c.run("-an");
}