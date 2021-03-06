/*
 *   Copyright (c) 2008-2019 SLIBIO <https://github.com/SLIBIO>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include "slib/social/oauth.h"

#include "slib/crypto/base64.h"
#include "slib/crypto/hmac.h"
#include "slib/crypto/sha1.h"
#include "slib/core/preference.h"
#include "slib/core/log.h"

#define TAG "OAuth"

namespace slib
{
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuth1_AccessToken)
	
	SLIB_DEFINE_JSON_MEMBERS(OAuth1_AccessToken, token, secret)
	
	OAuth1_AccessToken::OAuth1_AccessToken()
	{
	}
	
	OAuth1_AccessToken::OAuth1_AccessToken(const String& _token, const String& _tokenSecret)
	 : token(_token), secret(_tokenSecret)
	{
	}
	
	sl_bool OAuth1_AccessToken::isValid() const
	{
		return token.isNotEmpty() && secret.isNotEmpty();
	}
	
	void OAuth1_AccessToken::setResponse(const HashMap<String, String>& params)
	{
		token = params["oauth_token"];
		secret = params["oauth_token_secret"];
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuth1_AuthorizationRequestParam)
	
	OAuth1_AuthorizationRequestParam::OAuth1_AuthorizationRequestParam()
	{
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuth1_AccessTokenResult)
	
	OAuth1_AccessTokenResult::OAuth1_AccessTokenResult()
	{
		flagSuccess = sl_false;
	}
	
	void OAuth1_AccessTokenResult::setResponse(const HashMap<String, String>& params)
	{
		response = params;
		accessToken.setResponse(params);
		flagSuccess = accessToken.isValid();
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuth1_LoginResult)
	
	OAuth1_LoginResult::OAuth1_LoginResult()
	{
		flagCancel = sl_false;
		flagCache = sl_false;
	}
	
	void OAuth1_LoginResult::parseRedirectUrl(const String& _url)
	{
		Url url(_url);
		
		HashMap<String, String> params = HttpRequest::parseParameters(url.query);
		
		requestToken = params.getValue_NoLock("oauth_token");
		verifier  = params.getValue_NoLock("oauth_verifier");
		
		response = params;
		
		flagSuccess = requestToken.isNotEmpty() && verifier.isNotEmpty();
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuth1_Param)
	
	OAuth1_Param::OAuth1_Param()
	{
		requestTokenMethod = HttpMethod::POST;
		accessTokenMethod = HttpMethod::POST;
	}
	
	SLIB_DEFINE_OBJECT(OAuth1, Object)
	
	OAuth1::OAuth1(const OAuth1_Param& param)
	{
		m_preferenceName = param.preferenceName;
		restore();
		
		m_consumerKey = param.consumerKey;
		m_consumerSecret = param.consumerSecret;
		if (param.accessToken.isValid()) {
			m_accessToken = MakeShared<OAuth1_AccessToken>(param.accessToken);
		}
		
		m_requestTokenMethod = param.requestTokenMethod;
		m_requestTokenUrl = param.requestTokenUrl;
		m_authenticateUrl = param.authenticateUrl;
		m_accessTokenMethod = param.accessTokenMethod;
		m_accessTokenUrl = param.accessTokenUrl;
		m_callbackUrl = param.callbackUrl;
		m_flagLogErrors = sl_true;
	}
	
	OAuth1::~OAuth1()
	{
	}
	
	Ptr<OAuth1_AccessToken> OAuth1::getAccessToken()
	{
		return m_accessToken;
	}
	
	void OAuth1::setAccessToken(const OAuth1_AccessToken& accessToken)
	{
		if (accessToken.isValid()) {
			m_accessToken = MakeShared<OAuth1_AccessToken>(accessToken);
		} else {
			m_accessToken.setNull();
		}
		save();
	}
	
	void OAuth1::setAccessToken(const String& token, const String& secret)
	{
		if (token.isNotEmpty() && secret.isNotEmpty()) {
			m_accessToken = MakeShared<OAuth1_AccessToken>(token, secret);
		} else {
			m_accessToken.setNull();
		}
		save();
	}
	
	void OAuth1::clearAccessToken()
	{
		m_accessToken.setNull();
		save();
	}
	
	String OAuth1::getAccessTokenKey()
	{
		Ptr<OAuth1_AccessToken> accessToken = m_accessToken;
		if (accessToken.isNotNull()) {
			return accessToken->token;
		}
		return sl_null;
	}
	
	String OAuth1::getAccessTokenSecret()
	{
		Ptr<OAuth1_AccessToken> accessToken = m_accessToken;
		if (accessToken.isNotNull()) {
			return accessToken->secret;
		}
		return sl_null;
	}

	String OAuth1::getCallbackUrl()
	{
		return m_callbackUrl;
	}
	
	void OAuth1::setCallbackUrl(const String& url)
	{
		m_callbackUrl = url;
	}
	
	void OAuth1::setLoggingErrors(sl_bool flag)
	{
		m_flagLogErrors = flag;
	}
	
	String OAuth1::generateAuthorization(HttpMethod _method, const String& _url, HashMap<String, String>& parameters, const String& nonce, sl_int64 _timestamp, const String& token, const String& tokenSecret, const String& callbackUrl)
	{
		String consumerKey = Url::encodePercentByUTF8(m_consumerKey);
		String timestamp = String::fromInt64(_timestamp);

		Map< Pair<String, String>, sl_bool > mapParams;
		String method = HttpMethods::toString(_method);
		String url = _url;
		sl_reg indexQuery = url.indexOf('?');
		if (indexQuery > 0) {
			for (auto& item : HttpRequest::parseParameters(url.substring(indexQuery + 1))) {
				mapParams.add_NoLock(Pair<String, String>(Url::encodePercentByUTF8(item.key), Url::encodePercentByUTF8(item.value)), sl_true);
			}
			url = url.substring(0, indexQuery);
		}
		for (auto& item : parameters) {
			mapParams.add_NoLock(Pair<String, String>(Url::encodePercentByUTF8(item.key), Url::encodePercentByUTF8(item.value)), sl_true);
		}
		mapParams.add_NoLock(Pair<String, String>("oauth_consumer_key", consumerKey), sl_true);
		mapParams.add_NoLock(Pair<String, String>("oauth_nonce", nonce), sl_true);
		mapParams.add_NoLock(Pair<String, String>("oauth_signature_method", "HMAC-SHA1"), sl_true);
		mapParams.add_NoLock(Pair<String, String>("oauth_timestamp", timestamp), sl_true);
		mapParams.add_NoLock(Pair<String, String>("oauth_version", "1.0"), sl_true);
		if (token.isNotEmpty()) {
			mapParams.add_NoLock(Pair<String, String>("oauth_token", Url::encodePercentByUTF8(token)), sl_true);
		}
		if (callbackUrl.isNotEmpty()) {
			mapParams.add_NoLock(Pair<String, String>("oauth_callback", Url::encodePercentByUTF8(callbackUrl)), sl_true);
		}
		
		StringBuffer sbParams;
		{
			sl_bool flagFirstParam = sl_true;
			for (auto& item : mapParams) {
				if (flagFirstParam) {
					flagFirstParam = sl_false;
				} else {
					sbParams.addStatic("&", 1);
				}
				sbParams.add(item.key.first);
				sbParams.addStatic("=", 1);
				sbParams.add(item.key.second);
			}
		}
		
		StringBuffer sbSignature;
		sbSignature.add(method);
		sbSignature.addStatic("&", 1);
		sbSignature.add(Url::encodePercentByUTF8(url));
		sbSignature.addStatic("&", 1);
		sbSignature.add(Url::encodePercentByUTF8(sbParams.merge()));
		String signatureBase = sbSignature.merge();
		String signKey = Url::encodePercentByUTF8(m_consumerSecret) + "&";
		if (tokenSecret.isNotEmpty()) {
			signKey += Url::encodePercentByUTF8(tokenSecret);
		}
		char bufSignature[SHA1::HashSize];
		HMAC<SHA1>::execute(signKey.getData(), signKey.getLength(), signatureBase.getData(), signatureBase.getLength(), bufSignature);
		String signature = Url::encodePercentByUTF8(Base64::encode(bufSignature, SHA1::HashSize));

		mapParams.add_NoLock(Pair<String, String>("oauth_signature", signature), sl_true);
		StringBuffer sbAuthorization;
		{
			sbAuthorization.addStatic("OAuth ", 6);
			sl_bool flagFirstParam = sl_true;
			for (auto& item : mapParams) {
				if (item.key.first.startsWith("oauth_")) {
					if (flagFirstParam) {
						flagFirstParam = sl_false;
					} else {
						sbAuthorization.addStatic(", ", 2);
					}
					sbAuthorization.add(item.key.first);
					sbAuthorization.addStatic("=\"", 2);
					sbAuthorization.add(item.key.second);
					sbAuthorization.addStatic("\"", 1);
				}
			}
		}
		return sbAuthorization.merge();
	}
	
	void OAuth1::authorizeRequest(UrlRequestParam& param, const String& token, const String& tokenSecret, const String& callbackUrl)
	{
		char bufNonce[32];
		Math::randomMemory(bufNonce, 32);
		String nonce = String::makeHexString(bufNonce, 32);
		sl_int64 timestamp = Time::now().toUnixTime();

		HashMap<String, String> parameters;
		for (auto& item : param.parameters) {
			parameters.add_NoLock(item.key, item.value.getString());
		}
		
		if (param.method == HttpMethod::POST) {
			String type = param.requestHeaders.getValue(HttpHeaders::ContentType);
			sl_reg index = type.indexOf(';');
			if (index >= 0) {
				type = type.substring(0, index);
			}
			if (!(type.trim().equalsIgnoreCase(ContentTypes::MultipartFormData))) {
				if (param.requestBody.isNotNull()) {
					for (auto& item : HttpRequest::parseParameters(param.requestBody.getData(), param.requestBody.getSize())) {
						parameters.add_NoLock(item.key, item.value);
					}
				}
			}
		}
		
		String authorization = generateAuthorization(param.method, param.url, parameters, nonce, timestamp, token, tokenSecret, callbackUrl);
		param.requestHeaders.add_NoLock("Authorization", authorization);
	}
	
	void OAuth1::authorizeRequest(UrlRequestParam& param)
	{
		Ptr<OAuth1_AccessToken> token = m_accessToken;
		if (token.isNotNull()) {
			authorizeRequest(param, token->token, token->secret, sl_null);
		}
	}
	
	void OAuth1::getLoginUrl(const OAuth1_AuthorizationRequestParam& param, const Function<void(const String& url, const String& requestToken, const String& requestTokenSecret)>& onComplete)
	{
		String callbackUrl = param.callbackUrl;
		if (callbackUrl.isEmpty()) {
			callbackUrl = m_callbackUrl;
		}
		UrlRequestParam rp;
		rp.method = m_requestTokenMethod;
		rp.url = m_requestTokenUrl;
		if (rp.method == HttpMethod::POST) {
			rp.setRequestBodyAsHashMap(param.customParameters);
		} else {
			rp.parameters = param.customParameters;
		}
		auto thiz = ToRef(this);
		rp.onComplete = [thiz, onComplete](UrlRequest* request) {
			if (request->isError()) {
				thiz->logUrlRequestError(request);
				onComplete(sl_null, sl_null, sl_null);
				return;
			}
			String response = request->getResponseContentAsString();
			auto params = HttpRequest::parseParameters(response);
			String requestToken = params["oauth_token"];
			String requestTokenSecret = params["oauth_token_secret"];
			String loginUrl = params["login_url"];
			if (loginUrl.isEmpty()) {
				loginUrl = thiz->m_authenticateUrl.arg(Url::encodePercentByUTF8(requestToken));
			}
			onComplete(loginUrl, requestToken, requestTokenSecret);
		};
		authorizeRequest(rp, sl_null, sl_null, callbackUrl);
		UrlRequest::send(rp);
	}
	
	void OAuth1::getLoginUrl(const Function<void(const String& url, const String& requestToken, const String& requestTokenSecret)>& onComplete)
	{
		OAuth1_AuthorizationRequestParam param;
		getLoginUrl(param, onComplete);
	}
	
	void OAuth1::requestAccessToken(const String& verifier, const String& requestToken, const String& requestTokenSecret, const Function<void(OAuth1_AccessTokenResult&)>& onComplete)
	{
		UrlRequestParam rp;
		rp.method = m_accessTokenMethod;
		rp.url = m_accessTokenUrl;
		if (rp.method == HttpMethod::POST) {
			rp.setRequestBodyAsString("oauth_verifier=" + Url::encodeUriComponentByUTF8(verifier));
		} else {
			rp.parameters.put_NoLock("oauth_verifier", verifier);
		}
		auto thiz = ToRef(this);
		rp.onComplete = [thiz, onComplete](UrlRequest* request) {
			OAuth1_AccessTokenResult result;
			result.setResponse(HttpRequest::parseParameters(request->getResponseContentAsString()));
			if (request->isError()) {
				thiz->logUrlRequestError(request);
				result.flagSuccess = sl_false;
			}
			if (result.flagSuccess) {
				thiz->setAccessToken(result.accessToken);
			}
			onComplete(result);
		};
		
		authorizeRequest(rp, requestToken, requestTokenSecret, sl_null);
		UrlRequest::send(rp);
	}
	
	void OAuth1::logUrlRequestError(UrlRequest* request)
	{
		if (m_flagLogErrors) {
			LogError(TAG, "Error: %s, Status: %s, Response: %s", request->getLastErrorMessage(), HttpStatuses::toString(request->getResponseStatus()), request->getResponseContentAsString());
		}
	}
	
	void OAuth1::save()
	{
		if (m_preferenceName.isEmpty()) {
			return;
		}
		String key = "slib_oauth1_" + m_preferenceName;
		Json value;
		Ptr<OAuth1_AccessToken> accessToken = m_accessToken;
		if (accessToken.isNotNull()) {
			ToJson(value, *accessToken);
		}
		Preference::setValue(key, value);
	}
	
	void OAuth1::restore()
	{
		if (m_preferenceName.isEmpty()) {
			return;
		}
		String key = "slib_oauth1_" + m_preferenceName;
		Json value = Preference::getValue(key);
		if (value.isNotNull()) {
			OAuth1_AccessToken accessToken;
			FromJson(value, accessToken);
			m_accessToken = MakeShared<OAuth1_AccessToken>(accessToken);
		}
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuthAccessToken)
	
	SLIB_DEFINE_JSON_MEMBERS(OAuthAccessToken, token, refreshToken, tokenType, scopes, expirationTime, refreshTime)
	
	OAuthAccessToken::OAuthAccessToken()
	 : expirationTime(0), refreshTime(0)
	{}
	
	OAuthAccessToken::OAuthAccessToken(const String& _token)
	 : token(_token), expirationTime(0), refreshTime(Time::now())
	{}
	
	sl_bool OAuthAccessToken::isValid() const
	{
		return token.isNotEmpty() && (expirationTime.isZero() || Time::now() <= expirationTime);
	}
	
	sl_bool OAuthAccessToken::isValid(const List<String>& _requiredScopes) const
	{
		if (!(isValid())) {
			return sl_false;
		}
		ListElements<String> requiredScopes(_requiredScopes);
		for (sl_size i = 0; i < requiredScopes.count; i++) {
			if (!(scopes.contains(requiredScopes[i]))) {
				return sl_false;
			}
		}
		return sl_true;
	}
	
	void OAuthAccessToken::setResponse(const HashMap<String, String>& params)
	{
		token = params.getValue("access_token");
		if (token.isEmpty()) {
			return;
		}
		tokenType = params.getValue("token_type");
		refreshTime = Time::now();
		if (params.find("expires_in")) {
			sl_uint32 nSecondsExpiresIn = params.getValue("expires_in").parseUint32();
			expirationTime = refreshTime;
			expirationTime.addSeconds(nSecondsExpiresIn);
		} else {
			expirationTime.setZero();
		}
		refreshToken = params.getValue("refresh_token");
		scopes = params.getValue("scope").split(" ");
	}

	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuthAuthorizationRequestParam)
	
	OAuthAuthorizationRequestParam::OAuthAuthorizationRequestParam()
	{
		grantType = OAuthGrantType::Token;
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuthResult)
	
	OAuthResult::OAuthResult()
	{
		flagSuccess = sl_false;
		errorCode = OAuthErrorCode::None;
	}
	
	void OAuthResult::setResponse(const HashMap<String, String>& params)
	{
		flagSuccess = sl_false;
		error = params.getValue("error");
		errorDescription = params.getValue("error_description");
		errorUri = params.getValue("error_uri");
		if (error.isNotEmpty()) {
			if (error == "invalid_request") {
				errorCode = OAuthErrorCode::InvalidRequest;
			} else if (error == "unauthorized_client") {
				errorCode = OAuthErrorCode::UnauthorizedClient;
			} else if (error == "access_denied") {
				errorCode = OAuthErrorCode::AccessDenied;
			} else if (error == "unsupported_response_type") {
				errorCode = OAuthErrorCode::UnsupportedResponseType;
			} else if (error == "invalid_scope") {
				errorCode = OAuthErrorCode::InvalidScope;
			} else if (error == "server_error") {
				errorCode = OAuthErrorCode::ServerError;
			} else if (error == "temporarily_unavailable") {
				errorCode = OAuthErrorCode::TemporarilyUnavailable;
			} else if (error == "invalid_client") {
				errorCode = OAuthErrorCode::InvalidClient;
			} else if (error == "invalid_grant") {
				errorCode = OAuthErrorCode::InvalidGrant;
			} else if (error == "unsupported_grant_type") {
				errorCode = OAuthErrorCode::UnsupportedGrantType;
			} else {
				errorCode = OAuthErrorCode::None;
			}
		} else {
			flagSuccess = sl_true;
		}
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuthAccessTokenResult)
	
	OAuthAccessTokenResult::OAuthAccessTokenResult()
	{
	}
	
	void OAuthAccessTokenResult::setResponse(const Json& response)
	{
		this->response = response;
		HashMap<String, String> params;
		FromJson(response, params);
		OAuthResult::setResponse(params);
		accessToken.setResponse(params);
		if (!(accessToken.isValid())) {
			flagSuccess = sl_false;
		}
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuthLoginResult)
	
	OAuthLoginResult::OAuthLoginResult()
	{
		flagCancel = sl_false;
		flagCache = sl_false;
		errorCode = OAuthErrorCode::None;
	}
	
	void OAuthLoginResult::parseRedirectUrl(const String& _url)
	{
		Url url(_url);
		
		HashMap<String, String> params;
		params.addAll_NoLock(HttpRequest::parseParameters(url.query));
		params.addAll_NoLock(HttpRequest::parseParameters(url.fragment));
		
		OAuthResult::setResponse(params);
		flagCancel = errorCode == OAuthErrorCode::AccessDenied;

		accessToken.setResponse(params);
		
		state = params.getValue_NoLock("state");
		code = params.getValue_NoLock("code");
		
		response = params;
		
		if (code.isEmpty() && !(accessToken.isValid())) {
			flagSuccess = sl_false;
		}
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuthParam)
	
	OAuthParam::OAuthParam()
	{
		accessTokenMethod = HttpMethod::POST;
		flagUseBasicAuthorizationForAccessToken = sl_false;
		flagSupportTokenGrantType = sl_true;
		flagLoggingErrors = sl_true;
	}
	
	SLIB_DEFINE_OBJECT(OAuth2, Object)
	
	OAuth2::OAuth2(const OAuthParam& param)
	{
		m_preferenceName = param.preferenceName;
		restore();
		
		m_clientId = param.clientId;
		m_clientSecret = param.clientSecret;
		if (param.accessToken.isValid()) {
			m_accessToken = MakeShared<OAuthAccessToken>(param.accessToken);
		}
		
		m_authorizeUrl = param.authorizeUrl;
		m_accessTokenUrl = param.accessTokenUrl;
		m_accessTokenMethod = param.accessTokenMethod;
		m_flagUseBasicAuthorizationForAccessToken = param.flagUseBasicAuthorizationForAccessToken;
		m_redirectUri = param.redirectUri;
		m_loginRedirectUri = param.loginRedirectUri;
		m_defaultScopes = param.defaultScopes;
		m_flagSupportTokenGrantType = param.flagSupportTokenGrantType;
		
		m_flagLogErrors = param.flagLoggingErrors;
	}
	
	OAuth2::~OAuth2()
	{
	}
	
	Ptr<OAuthAccessToken> OAuth2::getAccessToken()
	{
		return m_accessToken;
	}
	
	void OAuth2::setAccessToken(const OAuthAccessToken& accessToken)
	{
		if (accessToken.isValid()) {
			m_accessToken = MakeShared<OAuthAccessToken>(accessToken);
		} else {
			m_accessToken.setNull();
		}
		save();
	}
	
	void OAuth2::setAccessToken(const String& accessToken)
	{
		if (accessToken.isNotEmpty()) {
			m_accessToken = MakeShared<OAuthAccessToken>(accessToken);
		} else {
			m_accessToken.setNull();
		}
		save();
	}
	
	void OAuth2::clearAccessToken()
	{
		m_accessToken.setNull();
		save();
	}
	
	String OAuth2::getAccessTokenKey()
	{
		Ptr<OAuthAccessToken> accessToken = m_accessToken;
		if (accessToken.isNotNull()) {
			return accessToken->token;
		}
		return sl_null;
	}
	
	List<String> OAuth2::getAccessTokenScopes()
	{
		Ptr<OAuthAccessToken> accessToken = m_accessToken;
		if (accessToken.isNotNull()) {
			return accessToken->scopes;
		}
		return sl_null;
	}
	
	void OAuth2::setLoggingErrors(sl_bool flag)
	{
		m_flagLogErrors = flag;
	}
	
	void OAuth2::authorizeRequest(UrlRequestParam& param, const OAuthAccessToken& token)
	{
		if (token.tokenType.isEmpty() || token.tokenType.equalsIgnoreCase("bearer")) {
			param.requestHeaders.put_NoLock("Authorization", "Bearer " + token.token);
		}
	}
	
	void OAuth2::authorizeRequest(UrlRequestParam& param)
	{
		Ptr<OAuthAccessToken> token = m_accessToken;
		if (token.isNotNull()) {
			authorizeRequest(param, *token);
		}
	}
	
	String OAuth2::getLoginUrl(const OAuthAuthorizationRequestParam& param)
	{
		Url url(m_authorizeUrl);
		auto oldParameters = url.getQueryParameters();
		HashMap<String, String> params;
		if (m_flagSupportTokenGrantType && param.grantType == OAuthGrantType::Token) {
			params.put_NoLock("response_type", "token");
		} else {
			params.put_NoLock("response_type", "code");
		}
		params.put_NoLock("client_id", m_clientId);
		String redirectUri = param.redirectUri;
		if (redirectUri.isEmpty()) {
			redirectUri = m_redirectUri;
		}
		if (redirectUri.isNotEmpty()) {
			params.put_NoLock("redirect_uri", redirectUri);
		}
		List<String> listScopes = param.scopes;
		if (listScopes.isNull()) {
			listScopes = m_defaultScopes;
		}
		String scope = StringBuffer::join(" ", listScopes).trim();
		if (scope.isNotEmpty()) {
			params.put_NoLock("scope", scope);
		}
		if (param.state.isNotEmpty()) {
			params.put_NoLock("state", param.state);
		}
		{
			MutexLocker lock(param.customParameters.getLocker());
			for (auto& item: param.customParameters) {
				params.put_NoLock(item.key, item.value.getString());
			}
		}
		params.putAll_NoLock(oldParameters);
		url.setQueryParameters(params);
		return url.toString();
	}
	
	String OAuth2::getLoginUrl(OAuthGrantType grantType, const List<String>& scopes, const String& state)
	{
		OAuthAuthorizationRequestParam param;
		param.grantType = grantType;
		param.scopes = scopes;
		param.state = state;
		return getLoginUrl(param);
	}
	
	String OAuth2::getLoginUrl(const List<String>& scopes, const String& state)
	{
		OAuthAuthorizationRequestParam param;
		param.scopes = scopes;
		param.state = state;
		return getLoginUrl(param);
	}
	
	void OAuth2::requestAccessToken(HashMap<String, Variant>& params, const Function<void(OAuthAccessTokenResult&)>& onComplete)
	{
		if (m_clientId.isEmpty() || m_clientSecret.isEmpty()) {
			OAuthAccessTokenResult result;
			onComplete(result);
			return;
		}
		UrlRequestParam rp;
		rp.method = m_accessTokenMethod;
		rp.url = m_accessTokenUrl;
		if (m_flagUseBasicAuthorizationForAccessToken) {
			rp.requestHeaders.put_NoLock("Authorization", "Basic " + Base64::encode(m_clientId + ":" + m_clientSecret));
		} else {
			params.put_NoLock("client_id", m_clientId);
			params.put_NoLock("client_secret", m_clientSecret);
		}
		if (rp.method == HttpMethod::POST) {
			rp.setRequestBodyAsHashMap(params);
		} else {
			rp.parameters = params;
		}
		auto thiz = ToRef(this);
		rp.onComplete = [thiz, onComplete](UrlRequest* request) {
			OAuthAccessTokenResult result;
			result.setResponse(request->getResponseContentAsJson());
			if (request->isError()) {
				thiz->logUrlRequestError(request);
				result.flagSuccess = sl_false;
			}
			if (result.flagSuccess) {
				thiz->setAccessToken(result.accessToken);
			}
			onComplete(result);
		};
		UrlRequest::send(rp);
	}
	
	void OAuth2::requestAccessToken(const Function<void(OAuthAccessTokenResult&)>& onComplete)
	{
		HashMap<String, Variant> params;
		params.put_NoLock("grant_type", "client_credentials");
		requestAccessToken(params, onComplete);
	}
	
	void OAuth2::requestAccessToken(const String& code, const String& redirectUri, const Function<void(OAuthAccessTokenResult&)>& onComplete)
	{
		HashMap<String, Variant> params;
		params.put_NoLock("grant_type", "authorization_code");
		params.put_NoLock("code", code);
		params.put_NoLock("redirect_uri", redirectUri);
		requestAccessToken(params, onComplete);
	}
	
	void OAuth2::requestAccessToken(const String& code, const Function<void(OAuthAccessTokenResult&)>& onComplete)
	{
		requestAccessToken(code, m_redirectUri, onComplete);
	}
	
	void OAuth2::requestRefreshToken(const String& refreshToken, const List<String>& scopes, const Function<void(OAuthAccessTokenResult&)>& onComplete)
	{
		HashMap<String, Variant> params;
		params.put_NoLock("grant_type", "refresh_token");
		params.put_NoLock("refresh_token", refreshToken);
		if (scopes.isNotNull()) {
			String s = StringBuffer::join(" ", scopes).trim();
			if (s.isNotEmpty()) {
				params.put_NoLock("scopes", s);
			}
		}
		requestAccessToken(params, onComplete);
	}
	
	void OAuth2::requestRefreshToken(const String& refreshToken, const Function<void(OAuthAccessTokenResult&)>& onComplete)
	{
		requestRefreshToken(refreshToken, sl_null, onComplete);
	}
	
	void OAuth2::logUrlRequestError(UrlRequest* request)
	{
		if (m_flagLogErrors) {
			LogError(TAG, "Error: %s, Status: %s, Response: %s", request->getLastErrorMessage(), HttpStatuses::toString(request->getResponseStatus()), request->getResponseContentAsString());
		}
	}
	
	void OAuth2::save()
	{
		if (m_preferenceName.isEmpty()) {
			return;
		}
		String key = "slib_oauth2_" + m_preferenceName;
		Json value;
		Ptr<OAuthAccessToken> accessToken = m_accessToken;
		if (accessToken.isNotNull()) {
			ToJson(value, *accessToken);
		}
		Preference::setValue(key, value);
	}
	
	void OAuth2::restore()
	{
		if (m_preferenceName.isEmpty()) {
			return;
		}
		String key = "slib_oauth2_" + m_preferenceName;
		Json value = Preference::getValue(key);
		if (value.isNotNull()) {
			OAuthAccessToken accessToken;
			FromJson(value, accessToken);
			m_accessToken = MakeShared<OAuthAccessToken>(accessToken);
		}
	}
	
	SLIB_DEFINE_CLASS_DEFAULT_MEMBERS(OAuthApiResult)
	
	OAuthApiResult::OAuthApiResult(UrlRequest* _request)
	{
		flagSuccess = sl_false;
		request = _request;
		if (_request) {
			response = _request->getResponseContentAsJson();
		}
	}
	
}
