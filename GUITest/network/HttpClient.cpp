#include <boost/url.hpp>
using namespace boost::urls;

#include "AsyncCallbacks.h"
#include "HttpClient.h"
#include <iostream>
#include <thread>

#include "root_certificates.hpp"

class HttpClient::GetData : public Callable
{
public:
	std::string url;
	GetCallback callback;
	void* userData = nullptr;
	std::thread* thread = nullptr;
	GetResult result;

	void call() override
	{
		thread->join();
		delete thread;
		callback(result, userData);
	}
};

HttpClient::HttpClient()
	: m_resolver(m_ioc)
	, m_ssl_ctx(ssl::context::tlsv12_client)
{
	load_root_certificates(m_ssl_ctx);
	m_ssl_ctx.set_verify_mode(ssl::context::verify_peer);
}

HttpClient::~HttpClient()
{

}

bool HttpClient::Get(const char* url, std::vector<unsigned char>& data)
{
	try
	{
		url_view uv = parse_uri(url).value();

		auto scheme = uv.scheme();		
		if (scheme == "")
		{
			scheme = "http";
		}
		auto host = uv.host();
		auto target = uv.encoded_path();
		auto port = uv.port();
		if (port == "")
		{
			if (scheme == "https")
			{
				port = "443";
			}
			else
			{
				port = "80";
			}
		}
		if (target == "")
		{
			target = "/";
		}

		auto const results = m_resolver.resolve(host, port);

		if (scheme == "https")
		{
			beast::ssl_stream<beast::tcp_stream> stream(m_ioc, m_ssl_ctx);

			if (!SSL_set_tlsext_host_name(stream.native_handle(), std::string(host).c_str()))
			{
				return false;
			}

			beast::get_lowest_layer(stream).connect(results);
			stream.handshake(ssl::stream_base::client);
			http::request<http::string_body> req{ http::verb::get, std::string(target).c_str(), 11 };
			req.set(http::field::host, std::string(host).c_str());
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			for (auto seq : res.body().data())
			{
				size_t start = data.size();
				size_t end = start + seq.size();
				data.resize(end);
				memcpy(data.data() + start, seq.data(), seq.size());
			}

			beast::error_code ec;
			stream.shutdown(ec);
			if (ec)
			{
				return false;
			}
			return true;
		}
		else
		{
			beast::tcp_stream stream(m_ioc);
			stream.connect(results);

			http::request<http::string_body> req{ http::verb::get, std::string(target).c_str(), 11 };
			req.set(http::field::host, std::string(host).c_str());
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			for (auto seq : res.body().data())
			{
				size_t start = data.size();
				size_t end = start + seq.size();
				data.resize(end);
				memcpy(data.data() + start, seq.data(), seq.size());
			}

			beast::error_code ec;
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);
			if (ec)
			{
				return false;
			}
			return true;
		}
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
	return false;
}

void HttpClient::GetThread(HttpClient* self, GetData* get_data)
{
	get_data->result.result = self->Get(get_data->url.c_str(), get_data->result.data);
	AsyncCallbacks::Add(get_data);
}

void HttpClient::GetAsync(const char* url, GetCallback callback, void* userData)
{
	GetData* get_data = new GetData;
	get_data->url = url;
	get_data->callback = callback;
	get_data->userData = userData;
	get_data->thread = new std::thread(GetThread, this, get_data);
}

bool HttpClient::GetHeaders(const char* url, std::unordered_map<std::string, std::string>& headers)
{
	try
	{
		url_view uv = parse_uri(url).value();
		auto scheme = uv.scheme();
		if (scheme == "")
		{
			scheme = "http";
		}
		auto host = uv.host();
		auto target = uv.encoded_path();
		auto port = uv.port();
		if (port == "")
		{
			if (scheme == "https")
			{
				port = "443";
			}
			else
			{
				port = "80";
			}
		}
		if (target == "")
		{
			target = "/";
		}

		auto const results = m_resolver.resolve(host, port);

		if (scheme == "https")
		{
			beast::ssl_stream<beast::tcp_stream> stream(m_ioc, m_ssl_ctx);

			if (!SSL_set_tlsext_host_name(stream.native_handle(), std::string(host).c_str()))
			{
				return false;
			}

			beast::get_lowest_layer(stream).connect(results);
			stream.handshake(ssl::stream_base::client);
			http::request<http::empty_body> req{ http::verb::head, std::string(target).c_str(), 11 };
			req.set(http::field::host, std::string(host).c_str());
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response_parser<http::empty_body> parser;
			parser.skip(true);

			http::read(stream, buffer, parser);

			auto heads = parser.get();
			auto iter = heads.begin();
			while (iter != heads.end())
			{
				std::string name(iter->name_string());
				std::string value(iter->value());
				headers[name] = value;
				iter++;
			}
		}
		else
		{			
			beast::tcp_stream stream(m_ioc);
			stream.connect(results);

			http::request<http::empty_body> req{ http::verb::head, std::string(target).c_str(), 11 };
			req.set(http::field::host, std::string(host).c_str());
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response_parser<http::empty_body> parser;
			parser.skip(true);
			
			http::read(stream, buffer, parser);

			auto heads = parser.get();
			auto iter = heads.begin();
			while (iter != heads.end())
			{
				std::string name(iter->name_string());
				std::string value(iter->value());
				headers[name] = value;
				iter++;
			}
		}

	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
	return false;
}

bool HttpClient::GetRange(const char* url, size_t offset, size_t size, std::vector<unsigned char>& data)
{
	try
	{
		url_view uv = parse_uri(url).value();

		auto scheme = uv.scheme();
		if (scheme == "")
		{
			scheme = "http";
		}
		auto host = uv.host();
		auto target = uv.encoded_path();
		auto port = uv.port();
		if (port == "")
		{
			if (scheme == "https")
			{
				port = "443";
			}
			else
			{
				port = "80";
			}
		}
		if (target == "")
		{
			target = "/";
		}

		auto const results = m_resolver.resolve(host, port);

		char s_range[64];
		sprintf(s_range, "bytes=%d-%d", offset, offset + size);

		if (scheme == "https")
		{
			beast::ssl_stream<beast::tcp_stream> stream(m_ioc, m_ssl_ctx);

			if (!SSL_set_tlsext_host_name(stream.native_handle(), std::string(host).c_str()))
			{
				return false;
			}

			beast::get_lowest_layer(stream).connect(results);
			stream.handshake(ssl::stream_base::client);
			http::request<http::string_body> req{ http::verb::get, std::string(target).c_str(), 11 };
			req.set(http::field::range, s_range);
			req.set(http::field::host, std::string(host).c_str());
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			for (auto seq : res.body().data())
			{
				size_t start = data.size();
				size_t end = start + seq.size();
				data.resize(end);
				memcpy(data.data() + start, seq.data(), seq.size());
			}

			beast::error_code ec;
			stream.shutdown(ec);
			if (ec)
			{
				return false;
			}
			return true;
		}
		else
		{
			beast::tcp_stream stream(m_ioc);
			stream.connect(results);

			http::request<http::string_body> req{ http::verb::get, std::string(target).c_str(), 11 };
			req.set(http::field::range, s_range);
			req.set(http::field::host, std::string(host).c_str());
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			for (auto seq : res.body().data())
			{
				size_t start = data.size();
				size_t end = start + seq.size();
				data.resize(end);
				memcpy(data.data() + start, seq.data(), seq.size());
			}

			beast::error_code ec;
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);
			if (ec)
			{
				return false;
			}
			return true;
		}
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
	return false;

}

