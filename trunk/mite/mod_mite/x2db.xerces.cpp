//____________________________________________________________________________
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <string.h>
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <stdlib.h>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include "sqlite3.h"
#include "x2db.h"

//____________________________________________________________________________
using namespace std;

//____________________________________________________________________________
X2DB::X2DB(request_rec *r, my_server_config *c): request(r), config(c),
stmt(NULL), index(0)
{
	return;
}

//____________________________________________________________________________
void X2DB::startElement
(const XMLCh * const uri, const XMLCh * const localname,
 const XMLCh * const qname, const Attributes &attributes)
{
	// todo: support nesting
	if (!strcmp("task", XMLString::transcode(localname))) {
		stmt = config->stmt[1];
		sqlite3_reset(stmt);
		ap_rputs("<statement>", request);
	} else if (stmt) {
		index = sqlite3_bind_parameter_index(stmt,
			XMLString::transcode(localname));
		ap_rprintf(request, "<column index=\"%d\" name=\"%s\">",
			index, XMLString::transcode(localname));
	}
	return;
}

//____________________________________________________________________________
void X2DB::endElement
(const XMLCh * const uri, const XMLCh * const localname,
 const XMLCh * const qname)
{
	if (!strcmp("task", XMLString::transcode(localname))) {
		ap_rputs("</statement>", request);
		sqlite3_step(stmt);
		stmt = NULL;
	} else if (stmt) {
		ap_rputs("</column>", request);
	}
	return;
}

//____________________________________________________________________________
void X2DB::characters(const XMLCh * const chars, const unsigned int length) {
	// todo: support multi-part text
	if (index > 0) {
		sqlite3_bind_text(stmt, index, XMLString::transcode(chars), -1, SQLITE_TRANSIENT);
		ap_rputs(XMLString::transcode(chars), request);
	}
	return;
}

//____________________________________________________________________________
void X2DB::error(const SAXParseException &e) {
	ap_rprintf(request,
		"<parse-diagnostic level=\"error\" file=\"%s\" line=\"%d\" "
		"column=\"%d\" message=\"%s\"/>",
		XMLString::transcode(e.getSystemId()),
		e.getLineNumber(),
		e.getColumnNumber(),
		XMLString::transcode(e.getMessage()));
	return;
}

//____________________________________________________________________________
void X2DB::fatalError(const SAXParseException &e) {
	ap_rprintf(request,
		"<parse-diagnostic level=\"fatal\" file=\"%s\" line=\"%d\" "
		"column=\"%d\" message=\"%s\"/>",
		XMLString::transcode(e.getSystemId()),
		e.getLineNumber(),
		e.getColumnNumber(),
		XMLString::transcode(e.getMessage()));
	return;
}

//____________________________________________________________________________
void X2DB::warning(const SAXParseException &e) {
	ap_rprintf(request,
		"<parse-diagnostic level=\"warning\" file=\"%s\" line=\"%d\" "
		"column=\"%d\" message=\"%s\"/>",
		XMLString::transcode(e.getSystemId()),
		e.getLineNumber(),
		e.getColumnNumber(),
		XMLString::transcode(e.getMessage()));
	return;
}

//____________________________________________________________________________
void X2DB::parse(request_rec *r, my_server_config *config, const char *buf,
				 apr_size_t len)
{
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException &x) {
		XERCES_STD_QUALIFIER cout << "Error during initialization! :\n"
			<< XMLString::transcode(x.getMessage()) << XERCES_STD_QUALIFIER endl;
		return;
	}
    SAX2XMLReader *parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
	parser->setFeature(XMLUni::fgXercesDynamic, true);
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);
    parser->setFeature(XMLUni::fgXercesSchema, true);
    parser->setFeature(XMLUni::fgXercesSchemaFullChecking, false);
    parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, false);
    int errorCode = 0;
	try {
		X2DB handler(r, config);
		parser->setContentHandler(&handler);
		parser->setErrorHandler(&handler);
		ap_rputs("<result>", r);
		parser->parse(MemBufInputSource(reinterpret_cast<XMLByte *>(const_cast<char *>(buf)), len, "HTTP"));
		ap_rprintf(r, "<parse-errors count=\"%d\"/>", parser->getErrorCount());
		ap_rputs("</result>", r);
	} catch (const OutOfMemoryException &) {
		XERCES_STD_QUALIFIER cout << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
		errorCode = 5;
	} catch (const XMLException &x) {
		XERCES_STD_QUALIFIER cout << "\nAn error occurred\n  Error: "
			<< XMLString::transcode(x.getMessage())
			<< "\n" << XERCES_STD_QUALIFIER endl;
		errorCode = 4;
	}
	if (errorCode) {
		XMLPlatformUtils::Terminate();
		return;
	}
	delete parser;
	XMLPlatformUtils::Terminate();
	return;
}

//__ EOF _____________________________________________________________________
