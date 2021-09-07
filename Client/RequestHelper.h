#pragma once

#pragma comment(lib, "Winhttp.lib")

#include "Logger.h"

#include <windows.h>
#include <WinHttp.h>
#include <string>
#include <mutex>

std::mutex requestM;

class RequestHelper
{
private:
	const std::wstring userAgent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36";
	//const std::wstring userAgent = L"Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) RiotClient/37.0.1 (CEF 74) Safari/537.36";
	const std::wstring getVerb = L"GET";
	const std::wstring postVerb = L"POST";
	const std::wstring patchVerb = L"PATCH";
	DWORD httpOptionIgnoreCert = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

	std::string unsafeGet(std::string url, std::string endPoint, int port, std::string authHeader = "")
	{
		HINTERNET hSession = NULL;
		HINTERNET hConnect = NULL;
		HINTERNET hRequest = NULL;
		BOOL bResults = FALSE;
		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		LPSTR pszOutBuffer;
		std::string rVal{ "" };

		hSession = WinHttpOpen(userAgent.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (hSession == NULL) return rVal;

		std::wstring wUrl(url.begin(), url.end());
		std::wstring wEndpoint(endPoint.begin(), endPoint.end());
		hConnect = WinHttpConnect(hSession, wUrl.c_str(), port, 0);
		if (hConnect == NULL)
		{
			std::string errorCode(std::to_string(GetLastError()));
			return rVal;
		}

		hRequest = WinHttpOpenRequest(hConnect, getVerb.c_str(), wEndpoint.c_str(),
			NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
		if (hRequest == NULL)
		{
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		if (!authHeader.empty())
		{
			std::wstring auth(authHeader.begin(), authHeader.end());
			auth.insert(0, L"Authorization: ");
			bResults = WinHttpAddRequestHeaders(hRequest,
				auth.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
			if (bResults == FALSE)
			{
				std::string errorCode(std::to_string(GetLastError()));
				if (hRequest) WinHttpCloseHandle(hRequest);
				if (hConnect) WinHttpCloseHandle(hConnect);
				if (hSession) WinHttpCloseHandle(hSession);
				return rVal;
			}
		}

		bResults = WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS,
			&httpOptionIgnoreCert, sizeof(httpOptionIgnoreCert));
		if (bResults == FALSE)
		{
			std::string errorCode(std::to_string(GetLastError()));
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		bResults = WinHttpSendRequest(
			hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
		if (bResults == FALSE)
		{
			std::string errorCode(std::to_string(GetLastError()));
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		bResults = WinHttpReceiveResponse(hRequest, NULL);
		if (bResults == FALSE)
		{
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		if (bResults)
		{
			do
			{
				dwSize = 0;
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				{
					break;
				}

				if (!dwSize)
					break;

				pszOutBuffer = new char[dwSize + 1];
				if (!pszOutBuffer)
				{
					break;
				}

				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
				{
					rVal.append(std::string(pszOutBuffer));
				}

				delete[] pszOutBuffer;

				if (!dwDownloaded)
					break;

			} while (dwSize > 0);
		}

		if (hRequest) WinHttpCloseHandle(hRequest);
		if (hConnect) WinHttpCloseHandle(hConnect);
		if (hSession) WinHttpCloseHandle(hSession);
		return rVal;
	}

	std::string unsafePost(std::string url, std::string endPoint, int port, std::string authHeader = "")
	{
		HINTERNET hSession = NULL;
		HINTERNET hConnect = NULL;
		HINTERNET hRequest = NULL;
		BOOL bResults = FALSE;
		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		LPSTR pszOutBuffer;
		std::string rVal{ "" };

		hSession = WinHttpOpen(userAgent.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (hSession == NULL) return rVal;

		std::wstring wUrl(url.begin(), url.end());
		std::wstring wEndpoint(endPoint.begin(), endPoint.end());
		hConnect = WinHttpConnect(hSession, wUrl.c_str(), port, 0);
		if (hConnect == NULL)
		{
			std::string errorCode(std::to_string(GetLastError()));
			return rVal;
		}

		hRequest = WinHttpOpenRequest(hConnect, postVerb.c_str(), wEndpoint.c_str(),
			NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
		if (hRequest == NULL)
		{
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		if (!authHeader.empty())
		{
			std::wstring auth(authHeader.begin(), authHeader.end());
			auth.insert(0, L"Authorization: ");
			bResults = WinHttpAddRequestHeaders(hRequest,
				auth.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
			if (bResults == FALSE)
			{
				std::string errorCode(std::to_string(GetLastError()));
				if (hRequest) WinHttpCloseHandle(hRequest);
				if (hConnect) WinHttpCloseHandle(hConnect);
				if (hSession) WinHttpCloseHandle(hSession);
				return rVal;
			}
		}

		bResults = WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS,
			&httpOptionIgnoreCert, sizeof(httpOptionIgnoreCert));
		if (bResults == FALSE)
		{
			std::string errorCode(std::to_string(GetLastError()));
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		bResults = WinHttpSendRequest(
			hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
		if (bResults == FALSE)
		{
			std::string errorCode(std::to_string(GetLastError()));
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		bResults = WinHttpReceiveResponse(hRequest, NULL);
		if (bResults == FALSE)
		{
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		if (bResults)
		{
			do
			{
				dwSize = 0;
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				{
					break;
				}

				if (!dwSize)
					break;

				pszOutBuffer = new char[dwSize + 1];
				if (!pszOutBuffer)
				{
					break;
				}

				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
				{

				}
				else
				{
					rVal.append(std::string(pszOutBuffer));
				}

				delete[] pszOutBuffer;

				if (!dwDownloaded)
					break;

			} while (dwSize > 0);
		}

		if (hRequest) WinHttpCloseHandle(hRequest);
		if (hConnect) WinHttpCloseHandle(hConnect);
		if (hSession) WinHttpCloseHandle(hSession);
		return rVal;
	}

	std::string unsafePatch(std::string url, std::string endPoint, int port, std::string authHeader = "", std::string body = "")
	{
		HINTERNET hSession = NULL;
		HINTERNET hConnect = NULL;
		HINTERNET hRequest = NULL;
		BOOL bResults = FALSE;
		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		LPSTR pszOutBuffer;
		std::string rVal{ "" };

		hSession = WinHttpOpen(userAgent.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (hSession == NULL) return rVal;

		std::wstring wUrl(url.begin(), url.end());
		std::wstring wEndpoint(endPoint.begin(), endPoint.end());
		hConnect = WinHttpConnect(hSession, wUrl.c_str(), port, 0);
		if (hConnect == NULL)
		{
			std::string errorCode(std::to_string(GetLastError()));
			return rVal;
		}

		hRequest = WinHttpOpenRequest(hConnect, patchVerb.c_str(), wEndpoint.c_str(),
			NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
		if (hRequest == NULL)
		{
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		if (!authHeader.empty())
		{
			std::wstring auth(authHeader.begin(), authHeader.end());
			auth.insert(0, L"Authorization: ");
			bResults = WinHttpAddRequestHeaders(hRequest,
				auth.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
			if (bResults == FALSE)
			{
				std::string errorCode(std::to_string(GetLastError()));
				if (hRequest) WinHttpCloseHandle(hRequest);
				if (hConnect) WinHttpCloseHandle(hConnect);
				if (hSession) WinHttpCloseHandle(hSession);
				return rVal;
			}
		}

		bResults = WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS,
			&httpOptionIgnoreCert, sizeof(httpOptionIgnoreCert));
		if (bResults == FALSE)
		{
			std::string errorCode(std::to_string(GetLastError()));
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		if (body.empty())
		{
			bResults = WinHttpSendRequest(
				hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
				WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
		}
		else
		{
			WriteLog("BODY: " + body);
			bResults = WinHttpSendRequest(
				hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
				(LPVOID)body.c_str(), body.size(), body.size(), 0);
		}
		if (bResults == FALSE)
		{
			std::string errorCode(std::to_string(GetLastError()));
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		bResults = WinHttpReceiveResponse(hRequest, NULL);
		if (bResults == FALSE)
		{
			if (hRequest) WinHttpCloseHandle(hRequest);
			if (hConnect) WinHttpCloseHandle(hConnect);
			if (hSession) WinHttpCloseHandle(hSession);
			return rVal;
		}

		if (bResults)
		{
			do
			{
				dwSize = 0;
				if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				{
					break;
				}

				if (!dwSize)
					break;

				pszOutBuffer = new char[dwSize + 1];
				if (!pszOutBuffer)
				{
					break;
				}

				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
				{

				}
				else
				{
					rVal.append(std::string(pszOutBuffer));
				}

				delete[] pszOutBuffer;

				if (!dwDownloaded)
					break;

			} while (dwSize > 0);
		}

		if (hRequest) WinHttpCloseHandle(hRequest);
		if (hConnect) WinHttpCloseHandle(hConnect);
		if (hSession) WinHttpCloseHandle(hSession);
		return rVal;
	}
public:

	/// <summary>
	/// Sends a GET request
	/// </summary>
	/// <param name="url">127.0.0.1</param>
	/// <param name="endPoint">/lol-client/test</param>
	/// <param name="port">1234</param>
	/// <returns>A response as string</returns>
	std::string Get(std::string url, std::string endPoint, int port, std::string authHeader = "")
	{
		WriteLog("GET -> " + endPoint);
		requestM.lock();
		std::string rVal = unsafeGet(url, endPoint, port, authHeader);
		requestM.unlock();
		WriteLog("RESPONSE -> " + rVal);
		return rVal;
	}

	/// <summary>
	/// Sends a POST request
	/// </summary>
	/// <param name="url">127.0.0.1</param>
	/// <param name="endPoint">/lol-client/test</param>
	/// <param name="port">1234</param>
	/// <returns>A response as string</returns>
	std::string Post(std::string url, std::string endPoint, int port, std::string authHeader = "")
	{
		WriteLog("POST -> " + endPoint);
		requestM.lock();
		std::string rVal = unsafePost(url, endPoint, port, authHeader);
		requestM.unlock();
		WriteLog("RESPONSE -> " + rVal);
		return rVal;
	}

	/// <summary>
	/// Sends a PATCH request
	/// </summary>
	/// <param name="url"></param>
	/// <param name="endPoint"></param>
	/// <param name="port"></param>
	/// <param name="authHeader"></param>
	/// <param name="body"></param>
	/// <returns></returns>
	std::string Patch(std::string url, std::string endPoint, int port, std::string authHeader = "", std::string body = "")
	{
		WriteLog("PATCH -> " + endPoint);
		requestM.lock();
		std::string rVal = unsafePatch(url, endPoint, port, authHeader, body);
		requestM.unlock();
		WriteLog("RESPONSE -> " + rVal);
		return rVal;
	}
};