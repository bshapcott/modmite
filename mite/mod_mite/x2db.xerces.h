#ifndef x2db_h
#define x2db_h

#include <xercesc/util/XMLChar.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include "httpd.h"
#include "http_protocol.h"
#include "http_config.h"
#include "serverconfig.h"

//____________________________________________________________________________
XERCES_CPP_NAMESPACE_USE

//____________________________________________________________________________
class X2DB: public DefaultHandler {
protected:
	request_rec *request;
	my_server_config *config;
	sqlite3_stmt *stmt;
	int index;
public:
	X2DB(request_rec *r, my_server_config *c);
	void startElement
		(const XMLCh * const uri, const XMLCh * const localname,
		const XMLCh * const qname, const Attributes &attributes);
	void endElement(const XMLCh * const uri,
		const XMLCh * const localname,
		const XMLCh * const qname);
	void characters(const XMLCh * const chars, const unsigned int length);
	void error(const SAXParseException &e);
	void fatalError(const SAXParseException &e);
	void warning(const SAXParseException &e);
	static void parse(request_rec *r, my_server_config *config,
		const char *buf, apr_size_t len);
};

#endif

//__ EOF _____________________________________________________________________
