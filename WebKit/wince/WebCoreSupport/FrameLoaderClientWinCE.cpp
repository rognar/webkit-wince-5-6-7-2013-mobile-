/*
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "FrameLoaderClientWinCE.h"

#include "DocumentLoader.h"
#include "FrameLoader.h"
#include "FrameNetworkingContextWinCE.h"
#include "FrameView.h"
#include "HTMLFormElement.h"
#include "HTMLEmbedElement.h"
#include "HTMLPlugInElement.h"
#include "MIMETypeRegistry.h"
#include "HTMLNames.h"
#include "NotImplemented.h"
#include "Page.h"
#include "PluginDatabase.h"
#include "RenderPart.h"
#include "SystemInfo.h"
#include "WebKitVersion.h"
#include "WebView.h"
#include <wtf/text/StringConcatenate.h>

using namespace WebCore;

namespace WebKit {

FrameLoaderClientWinCE::FrameLoaderClientWinCE(WebView* view)
    : m_webView(view)
    , m_pluginView(0)
	, m_floadingCount(0)
{
    ASSERT(m_webView);
}

FrameLoaderClientWinCE::~FrameLoaderClientWinCE()
{
}

String FrameLoaderClientWinCE::userAgent(const KURL&)
{
    DEFINE_STATIC_LOCAL(String, userAgentString, ());

    if (userAgentString.isNull()) {
        String webKitVersion = String::format("%d.%d", WEBKIT_MAJOR_VERSION, WEBKIT_MINOR_VERSION);
        userAgentString = makeString("Mozilla/5.0 (", windowsVersionForUAString(),") AppleWebKit/", webKitVersion, " (KHTML, like Gecko) Mobile Safari/", webKitVersion);
    }

    return userAgentString;
}

PassRefPtr<DocumentLoader> FrameLoaderClientWinCE::createDocumentLoader(const WebCore::ResourceRequest& request, const SubstituteData& substituteData)
{
    return DocumentLoader::create(request, substituteData);
}

void FrameLoaderClientWinCE::committedLoad(DocumentLoader* loader, const char* data, int length)
{
    if (m_pluginView) {
        if (!m_hasSentResponseToPlugin) {
            m_pluginView->didReceiveResponse(loader->response());
            m_hasSentResponseToPlugin = true;
        }
        m_pluginView->didReceiveData(data, length);
    } else
        loader->commitData(data, length);
}

bool FrameLoaderClientWinCE::shouldUseCredentialStorage(DocumentLoader*, unsigned long)
{
    notImplemented();
    return false;
}

void FrameLoaderClientWinCE::dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&)
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidCancelAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&)
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchWillSendRequest(DocumentLoader*, unsigned long, WebCore::ResourceRequest&, const WebCore::ResourceResponse&)
{
    notImplemented();
}

void FrameLoaderClientWinCE::assignIdentifierToInitialRequest(unsigned long, DocumentLoader*, const WebCore::ResourceRequest&)
{
    notImplemented();
}

void FrameLoaderClientWinCE::postProgressStartedNotification()
{
    notImplemented();
}

void FrameLoaderClientWinCE::postProgressEstimateChangedNotification()
{
    notImplemented();
}

void FrameLoaderClientWinCE::postProgressFinishedNotification()
{
    notImplemented();
}

void FrameLoaderClientWinCE::frameLoaderDestroyed()
{
    m_webView = 0;
    m_frame = 0;
    delete this;
}

void FrameLoaderClientWinCE::dispatchDidReceiveResponse(DocumentLoader*, unsigned long, const ResourceResponse& response)
{
    m_response = response;
/*	if(WebView::m_CallBackFuncs[EEID_NAVIGATECOMPLETE])
		WebView::m_CallBackFuncs[EEID_NAVIGATECOMPLETE](EEID_NAVIGATECOMPLETE, (LPARAM)(response.url().string().utf8().data()), 0);*/
}

void FrameLoaderClientWinCE::dispatchDecidePolicyForResponse(FramePolicyFunction policyFunction, const WebCore::ResourceResponse& response, const WebCore::ResourceRequest&)
{
    if (canShowMIMEType(response.mimeType()))
        (m_frame->loader()->policyChecker()->*policyFunction)(PolicyUse);
    else
        (m_frame->loader()->policyChecker()->*policyFunction)(PolicyDownload);
}

void FrameLoaderClientWinCE::dispatchDecidePolicyForNewWindowAction(FramePolicyFunction policyFunction, const NavigationAction&, const WebCore::ResourceRequest&, PassRefPtr<FormState>, const String&)
{
    (m_frame->loader()->policyChecker()->*policyFunction)(PolicyUse);
}

void FrameLoaderClientWinCE::dispatchDecidePolicyForNavigationAction(FramePolicyFunction policyFunction, const NavigationAction&, const WebCore::ResourceRequest&, PassRefPtr<FormState>)
{
    (m_frame->loader()->policyChecker()->*policyFunction)(PolicyUse);
}

void FrameLoaderClientWinCE::dispatchWillSubmitForm(FramePolicyFunction policyFunction, PassRefPtr<FormState>)
{
    (m_frame->loader()->policyChecker()->*policyFunction)(PolicyUse);
}

PassRefPtr<Widget> FrameLoaderClientWinCE::createPlugin(const WebCore::IntSize& pluginSize, WebCore::HTMLPlugInElement* element, const WebCore::KURL& url, const WTF::Vector<WTF::String>& paramNames, const WTF::Vector<WTF::String>& paramValues, const WTF::String& mimeType, bool loadManually)
{
    RefPtr<PluginView> pluginView = PluginView::create(m_frame, pluginSize, element, url, paramNames, paramValues, mimeType, loadManually);

    if (pluginView->status() == PluginStatusLoadedSuccessfully)
        return pluginView;

    dispatchDidFailToStartPlugin(pluginView.get());

    return 0;
}

PassRefPtr<Frame> FrameLoaderClientWinCE::createFrame(const KURL& url, const String& name, HTMLFrameOwnerElement* ownerElement,
                                                 const String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
{
    return m_webView->createFrame(url, name, ownerElement, referrer, allowsScrolling, marginWidth, marginHeight);
}

void FrameLoaderClientWinCE::redirectDataToPlugin(Widget* pluginWidget)
{
    m_pluginView = toPluginView(pluginWidget);
    if (pluginWidget)
        m_hasSentResponseToPlugin = false;
}

PassRefPtr<Widget> FrameLoaderClientWinCE::createJavaAppletWidget(const IntSize&, HTMLAppletElement*, const KURL&, const Vector<String>&, const Vector<String>&)
{
    notImplemented();
    return 0;
}

ObjectContentType FrameLoaderClientWinCE::objectContentType(const KURL& url, const String& mimeType, bool shouldPreferPlugInsForImages)
{
    return FrameLoader::defaultObjectContentType(url, mimeType, shouldPreferPlugInsForImages);
}

String FrameLoaderClientWinCE::overrideMediaType() const
{
    notImplemented();
    return String();
}

void FrameLoaderClientWinCE::dispatchDidClearWindowObjectInWorld(DOMWrapperWorld*)
{
    notImplemented();
}

void FrameLoaderClientWinCE::documentElementAvailable()
{
    notImplemented();
}

void FrameLoaderClientWinCE::didPerformFirstNavigation() const
{
    //navcomplete
	notImplemented();
}

void FrameLoaderClientWinCE::registerForIconNotification(bool)
{
    notImplemented();
}

void FrameLoaderClientWinCE::setMainFrameDocumentReady(bool)
{
    notImplemented();
}

bool FrameLoaderClientWinCE::hasWebView() const
{
    return true;
}

void FrameLoaderClientWinCE::dispatchDidFinishLoad()
{
/*	if(WebView::m_CallBackFuncs[EEID_DOCUMENTCOMPLETE])
		WebView::m_CallBackFuncs[EEID_DOCUMENTCOMPLETE](EEID_DOCUMENTCOMPLETE, (LPARAM)(m_frame->loader()->activeDocumentLoader()->url().string().utf8().data()), 0);*/
}

void FrameLoaderClientWinCE::frameLoadCompleted()
{
    notImplemented();//podavat na vkhod funkciuu kak registerevent i iz dll exportit
}

void FrameLoaderClientWinCE::saveViewStateToItem(HistoryItem*)
{
    notImplemented();
}

void FrameLoaderClientWinCE::restoreViewState()
{
    notImplemented();
}

bool FrameLoaderClientWinCE::shouldGoToHistoryItem(HistoryItem* item) const
{
    return item;
}

bool FrameLoaderClientWinCE::shouldStopLoadingForHistoryItem(HistoryItem* item) const
{
    return true;
}

void FrameLoaderClientWinCE::didDisplayInsecureContent()
{
    notImplemented();
}

void FrameLoaderClientWinCE::didRunInsecureContent(SecurityOrigin*, const KURL&)
{
    notImplemented();
}

void FrameLoaderClientWinCE::didDetectXSS(const KURL&, bool)
{
    notImplemented();
}

void FrameLoaderClientWinCE::makeRepresentation(DocumentLoader*)
{
    notImplemented();
}

void FrameLoaderClientWinCE::forceLayout()
{
    notImplemented();
}

void FrameLoaderClientWinCE::forceLayoutForNonHTML()
{
    notImplemented();
}

void FrameLoaderClientWinCE::setCopiesOnScroll()
{
    notImplemented();
}

void FrameLoaderClientWinCE::detachedFromParent2()
{
    notImplemented();
}

void FrameLoaderClientWinCE::detachedFromParent3()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidHandleOnloadEvents()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidCancelClientRedirect()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchWillPerformClientRedirect(const KURL&, double, double)
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidChangeLocationWithinPage()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidPushStateWithinPage()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidReplaceStateWithinPage()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidPopStateWithinPage()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchWillClose()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidReceiveIcon()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidStartProvisionalLoad()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidReceiveTitle(const StringWithDirection& value)
{
	if(WebView::m_CallBackFuncs[EEID_TITLECHANGE])
		WebView::m_CallBackFuncs[EEID_TITLECHANGE](EEID_TITLECHANGE, (LPARAM)(value.string().utf8().data()), 0);
}

void FrameLoaderClientWinCE::dispatchDidChangeIcons(WebCore::IconType)
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidCommitLoad()
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidFinishDocumentLoad()
{
    RefPtr<Element> body = m_frame->document()->body();
	RefPtr<Element> embedElement = m_frame->document()->createElement(HTMLNames::embedTag, false);
	HTMLEmbedElement* m_embedElement;
	m_embedElement = static_cast<HTMLEmbedElement*>(embedElement.get());
	m_embedElement->setAttribute(HTMLNames::nameAttr, "rhobridge");
	m_embedElement->setAttribute(HTMLNames::srcAttr, "");
	m_embedElement->setAttribute(HTMLNames::typeAttr, "application/x-rho-bridge");
	body->appendChild(embedElement, IGNORE_EXCEPTION);
}

void FrameLoaderClientWinCE::dispatchDidLayout(LayoutMilestones)
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchShow()
{
    notImplemented();
}

void FrameLoaderClientWinCE::cancelPolicyCheck()
{
    notImplemented();
}

void FrameLoaderClientWinCE::revertToProvisionalState(DocumentLoader*)
{
    notImplemented();
}

void FrameLoaderClientWinCE::willChangeTitle(DocumentLoader*)
{
    notImplemented();
}

void FrameLoaderClientWinCE::didChangeTitle(DocumentLoader* documentLoader)
{
    setTitle(documentLoader->title(), documentLoader->url());
}

bool FrameLoaderClientWinCE::canHandleRequest(const WebCore::ResourceRequest&) const
{
    notImplemented();
    return true;
}

bool FrameLoaderClientWinCE::canShowMIMEType(const String& type) const
{
    return (MIMETypeRegistry::canShowMIMEType(type)
            || PluginDatabase::installedPlugins()->isMIMETypeRegistered(type));
}

bool FrameLoaderClientWinCE::canShowMIMETypeAsHTML(const String&) const
{
    notImplemented();
    return false;
}

bool FrameLoaderClientWinCE::representationExistsForURLScheme(const String&) const
{
    notImplemented();
    return false;
}

String FrameLoaderClientWinCE::generatedMIMETypeForURLScheme(const String&) const
{
    notImplemented();
    return String();
}

void FrameLoaderClientWinCE::finishedLoading(DocumentLoader*)
{
	m_floadingCount++;
    
	if(m_floadingCount == 2)
	{
		/*WTF::Vector<WTF::String> paramNames;
		paramNames.append("id");
		WTF::Vector<WTF::String> paramValues;
		paramValues.append("rhobridgeId");
		PassRefPtr<HTMLElement> htmlElement = WebCore::HTMLEmbedElement::create(HTMLNames::embedTag, m_frame->document(), false);
		m_frame->view()->addWidgetToUpdate(*/
		//pluginElement.get()->setRenderer(m_frame->document()->renderer());
        //HTMLPlugInImageElement* pluginElement = (HTMLPlugInImageElement*)(htmlElement.get());
		//RenderObject* newRenderer = pluginElement.get()->elementData()-> element->createRenderer(document->renderArena(), m_style.get());
		//m_frame->loader()->subframeLoader()->requestObject(pluginElement, "test.rho", WTF::nullAtom, "application/x-rho-bridge", paramNames, paramValues);
		//mozhet nado eshe parent element ukazat
		/*PluginDatabase::installedPlugins()->refresh();
		const Vector<PluginPackage*>& plugins = PluginDatabase::installedPlugins()->plugins();
		for (size_t i = 0; i < plugins.size(); ++i) {
			PluginPackage* package = plugins[i];
			package->load();
			NPP_t instance;
			NPError npErr = package->pluginFuncs()->newp((NPMIMEType)"application/x-symbol-bridge", &instance, 0, 0, NULL, NULL , NULL);
		}*/

		/*IntSize rect;
		WebCore::HTMLPlugInElement pelement;
		m_pluginView = PluginView::create(m_frame, rect, */
		/*WTF::Vector<WTF::String> paramNames;
		paramNames.append("id");
		WTF::Vector<WTF::String> paramValues;
		paramValues.append("rhobridgeId");*/
		/*PassRefPtr<Element> embed =  Element::create(HTMLNames::embedTag, m_frame->document()); 
		embed.get()->setAttribute(HTMLNames::typeAttr, "application/x-rho-bridge");
		embed.get()->setAttribute(HTMLNames::idAttr, "embed1");
		m_frame->document()->appendChild(embed);*/
		//embed.get()->
		//m_frame->loader()->subframeLoader()->requestObject((HTMLPlugInImageElement*)(embed.get()), "test.rho", WTF::nullAtom, "application/x-rho-bridge", paramNames, paramValues);*/
		/*PassRefPtr<Element> embed =  Element::create(HTMLNames::embedTag, m_frame->document()); 
		embed.get()->setAttribute(HTMLNames::typeAttr, "application/x-rho-bridge");
		embed.get()->setAttribute(HTMLNames::idAttr, "embed1");
		embed.get()->setAttribute(HTMLNames::srcAttr, "test.embed");*/
		//m_frame->document()->appendChild(embed);

		/*RefPtr<Element> body = m_frame->document()->body();
		RefPtr<Element> embedElement = m_frame->document()->createElement(HTMLNames::embedTag, false);
		HTMLEmbedElement* m_embedElement;
		m_embedElement = static_cast<HTMLEmbedElement*>(embedElement.get());
		m_embedElement->setAttribute(HTMLNames::nameAttr, "rhobridge");
		m_embedElement->setAttribute(HTMLNames::srcAttr, "rhobridge");
		m_embedElement->setAttribute(HTMLNames::typeAttr, "application/x-rho-bridge");
		body->appendChild(embedElement, IGNORE_EXCEPTION);*/
	}
	
	if (!m_pluginView)
        return;

    m_pluginView->didFinishLoading();
    m_pluginView = 0;
    m_hasSentResponseToPlugin = false;
}

void FrameLoaderClientWinCE::provisionalLoadStarted()
{
    notImplemented();
}

void FrameLoaderClientWinCE::didFinishLoad()
{
    notImplemented();
}

void FrameLoaderClientWinCE::prepareForDataSourceReplacement()
{
    notImplemented();
}

void FrameLoaderClientWinCE::setTitle(const StringWithDirection&, const KURL&)
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidReceiveContentLength(DocumentLoader*, unsigned long, int)
{
    notImplemented();
}

void FrameLoaderClientWinCE::dispatchDidFinishLoading(DocumentLoader*, unsigned long)
{
}

void FrameLoaderClientWinCE::dispatchDidFailLoading(DocumentLoader*, unsigned long, const ResourceError&)
{
    /*if(WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR])
		WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR](EEID_NAVIGATIONERROR, (LPARAM)(m_frame->loader()->activeDocumentLoader()->url().string().utf8().data()), 0);*/
}
/*NADO otrkyt doku ne server lokalno svoem i tam ssylku dat bituu i proverit. vidimo webview tut ne rbaotaet tak kak metod load ne dergaetsya
takzhe nado kogda nash http serve rotdaet oshibku tozhe pokazyvat badlink i nado timeout sdelat obyazatelno
i razobratd  stya s rename i proeveri ego vnachale na steibl 36... khitorzhopyi webkit nado eventy pokhodu bolee gluboko vtykat
prichem po taimautu nado i nash http server obrabatyvat i vneshie i nado proverit zagruzku po navigate i zagruzku po nazhatiu ssylku
navigate eto vidimo setmaindocerro ili fallback, a po ssylke drugoe... no ya dumau tam est obshii mekhnizm on dolzhen byt i na taimout obshii tozhe!!! documentloder!!!
ottua pryamo static membery webview vyzyvat i fairit eventy v rhodes!!!
 i nado vsetak ina dok complete vozmozhno uzhe tolko v master zalit na didfinisedload i naiti eshe kuda vstavit navcomplete, potom mozhet eshe beforenav, no eto tolko v master
 v steibl tolko obrabotka naveror, navtimeout!!!!!!!!!!!!!!!!!!!!!!!!!!!! a u nas obshaya repa rhotauextensions tuda vse voidet, nado dogda eti doc nav before complete protestit 
 tshatelno zavtra!!! Ya ponyal webkit prosto padaet koroche nado logi togda suda vstavit!! a to tak ne ponyatno!!! ili messgae box*/
bool FrameLoaderClientWinCE::dispatchDidLoadResourceFromMemoryCache(DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int)
{
    notImplemented();
    return false;
}

void FrameLoaderClientWinCE::dispatchDidFailProvisionalLoad(const ResourceError& error)
{
    dispatchDidFailLoad(error);
}

void FrameLoaderClientWinCE::dispatchDidFailLoad(const ResourceError&)
{
  /*  if(WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR])
		WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR](EEID_NAVIGATIONERROR, (LPARAM)(m_frame->loader()->activeDocumentLoader()->url().string().utf8().data()), 0);*/
}

void FrameLoaderClientWinCE::convertMainResourceLoadToDownload(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&)
{
    notImplemented();
}

ResourceError FrameLoaderClientWinCE::cancelledError(const WebCore::ResourceRequest&)
{
    return ResourceError();
}

ResourceError FrameLoaderClientWinCE::blockedError(const WebCore::ResourceRequest&)
{
    return ResourceError();
}

ResourceError FrameLoaderClientWinCE::cannotShowURLError(const WebCore::ResourceRequest&)
{
    return ResourceError();
}

ResourceError FrameLoaderClientWinCE::interruptedForPolicyChangeError(const WebCore::ResourceRequest&)
{
    return ResourceError();
}

ResourceError FrameLoaderClientWinCE::cannotShowMIMETypeError(const WebCore::ResourceResponse&)
{
    return ResourceError();
}

ResourceError FrameLoaderClientWinCE::fileDoesNotExistError(const WebCore::ResourceResponse&)
{
    return ResourceError();
}

ResourceError FrameLoaderClientWinCE::pluginWillHandleLoadError(const WebCore::ResourceResponse&)
{
    return ResourceError();
}

bool FrameLoaderClientWinCE::shouldFallBack(const ResourceError& error)
{	
	/*if(WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR])
		WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR](EEID_NAVIGATIONERROR, (LPARAM)(m_frame->loader()->activeDocumentLoader()->url().string().utf8().data()), 0);*/
	return !(error.isCancellation());
}

bool FrameLoaderClientWinCE::canCachePage() const
{
    return true;
}

Frame* FrameLoaderClientWinCE::dispatchCreatePage(const NavigationAction&)
{
    notImplemented();
    return 0;
}

void FrameLoaderClientWinCE::dispatchUnableToImplementPolicy(const ResourceError&)
{
    notImplemented();
}

void FrameLoaderClientWinCE::setMainDocumentError(DocumentLoader*, const ResourceError& error)
{	
	/*if(WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR])
		WebView::m_CallBackFuncs[EEID_NAVIGATIONERROR](EEID_NAVIGATIONERROR, (LPARAM)(m_frame->loader()->activeDocumentLoader()->url().string().utf8().data()), 0);*/
	
	if (!m_pluginView)
        return;

    m_pluginView->didFail(error);
    m_pluginView = 0;
    m_hasSentResponseToPlugin = false;
}

void FrameLoaderClientWinCE::startDownload(const WebCore::ResourceRequest&, const String& /* suggestedName */)
{
    //navcomplete
	notImplemented();
}

void FrameLoaderClientWinCE::updateGlobalHistory()
{
    notImplemented();
}

void FrameLoaderClientWinCE::updateGlobalHistoryRedirectLinks()
{
    notImplemented();
}

void FrameLoaderClientWinCE::savePlatformDataToCachedFrame(CachedFrame*)
{
    notImplemented();
}

void FrameLoaderClientWinCE::transitionToCommittedFromCachedFrame(CachedFrame*)
{
    notImplemented();
}

void FrameLoaderClientWinCE::transitionToCommittedForNewPage()
{
    Page* page = m_frame->page();
    ASSERT(page);

    bool isMainFrame = m_frame == page->mainFrame();

    m_frame->setView(0);

    RefPtr<FrameView> frameView;
    if (isMainFrame) {
        RECT rect;
        m_webView->frameRect(&rect);
        frameView = FrameView::create(m_frame, IntRect(rect).size());
    } else
        frameView = FrameView::create(m_frame);

    m_frame->setView(frameView);

    if (m_frame->ownerRenderer())
        m_frame->ownerRenderer()->setWidget(frameView);
}

void FrameLoaderClientWinCE::didSaveToPageCache()
{
}

void FrameLoaderClientWinCE::didRestoreFromPageCache()
{
}

void FrameLoaderClientWinCE::dispatchDidBecomeFrameset(bool)
{
}

PassRefPtr<WebCore::FrameNetworkingContext> FrameLoaderClientWinCE::createNetworkingContext()
{
    return FrameNetworkingContextWinCE::create(m_frame, userAgent(KURL()));
}

} // namespace WebKit
