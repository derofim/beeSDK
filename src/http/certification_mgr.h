#ifndef __CERTIFICATION_MGR_H__
#define __CERTIFICATION_MGR_H__

#ifdef ENABLE_HTTPS

#include "utility/common.h"
#include <boost/asio/ssl.hpp>

namespace bee {

class CertificationMgr
	: private boost::noncopyable
	, public std::enable_shared_from_this<CertificationMgr>
{
public:
	static std::shared_ptr<CertificationMgr> inst()
	{
		if (s_pinst_ == NULL)
		{
			boost::mutex::scoped_lock lock(instance_mutex_);
			if (s_pinst_ == NULL)
			{
				s_pinst_.reset(new CertificationMgr);
			}
		}
		return s_pinst_;
	}

	static void destroy()
	{
		if (s_pinst_ != NULL)
		{
			boost::mutex::scoped_lock lock(instance_mutex_);
			if (s_pinst_ != NULL)
			{
				s_pinst_.reset();
			}
		}
	}

	CertificationMgr() : ssl_ctx_(boost::asio::ssl::context::sslv23) {}
	~CertificationMgr() {}

	void init();

	boost::asio::ssl::context&	ssl_context() { return ssl_ctx_; }
private:
	static std::shared_ptr<CertificationMgr> s_pinst_;
	static boost::mutex instance_mutex_;

	boost::asio::ssl::context	ssl_ctx_;
	std::string		cer_chain_path_;
};

} // namespace bee

#endif  //ENABLE_HTTPS
#endif
