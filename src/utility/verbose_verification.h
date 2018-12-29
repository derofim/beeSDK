﻿#ifndef	__VERBOSE_VERIFICATION_H__
#define	__VERBOSE_VERIFICATION_H__

#ifdef ENABLE_HTTPS

#include "common.h"
#include <boost/asio/ssl.hpp>

namespace bee {

///@brief Helper class that prints the current certificate's subject
///       name and the verification results.
template <typename Verifier>
class verbose_verification
{
public:
	verbose_verification(Verifier verifier)
		: verifier_(verifier)
	{}

	bool operator()(
		bool preverified,
		boost::asio::ssl::verify_context& ctx
		)
	{
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
		bool verified = verifier_(preverified, ctx);
		std::string str(subject_name);
		return verified;
	}
private:
	Verifier verifier_;
};

///@brief Auxiliary function to make verbose_verification objects.
template <typename Verifier>
verbose_verification<Verifier>
make_verbose_verification(Verifier verifier)
{
	return verbose_verification<Verifier>(verifier);
}

} // namespace bee

#endif //ENABLE_HTTPS
#endif
