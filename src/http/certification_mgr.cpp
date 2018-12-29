#include "certification_mgr.h"

#ifdef ENABLE_HTTPS

namespace bee {

std::shared_ptr<CertificationMgr>     CertificationMgr::s_pinst_;
boost::mutex CertificationMgr::instance_mutex_;

void CertificationMgr::init()
{
	//现在直接用现成的根+中间证书链的pem, 以后可以考虑各平台自己的api，获取操作系统自带的pem
	//cer_chain_path_ = "D:\\SVN-Code\\SohuAccelerator\\p2p\\windows\\bin\\Debug\\symantec_roots.pem";
	cer_chain_path_ = "";
	if (cer_chain_path_.empty() == false)
		ssl_ctx_.load_verify_file(cer_chain_path_);
}

} // namespace bee

#endif
