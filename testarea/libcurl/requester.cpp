#include "requester.h"

#include <curl/curl.h>
#include <pthread.h>
#include <map>
#include <string>
#include <vector>

struct Requester
{
	struct DownloadData
	{
		DownloadData(const char* url, req_callback_type callback, void* callbackdata) : url(url), finished(false), aborted(false), tobedeleted(false), callback(callback), callbackdata(callbackdata) {}

		std::string url;
		pthread_t thread;
		std::vector<char> data;
		bool finished;
		bool aborted;
		bool tobedeleted;
		req_callback_type callback;
		void* callbackdata;
		CURLcode res;
	};

	std::map<int, DownloadData*> activedata;
	std::map<int, DownloadData*> finisheddata;

	int downloadId;
};

Requester g_requester;

static size_t writeMemoryCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
	Requester::DownloadData* dd = static_cast<Requester::DownloadData*>(data);
	if (dd->aborted)
		return 0;

//	printf("w");
	size_t realsize = size * nmemb;

	size_t curr = dd->data.size();
	dd->data.resize(curr + realsize);
	memcpy(&dd->data[curr], ptr, realsize);

	return realsize;
}

static int progressCallback(void *ptr, double dltotal, double dlnow, double ult, double uln)
{
	Requester::DownloadData* dd = static_cast<Requester::DownloadData*>(ptr);
	if (dd->aborted)
		return 1;

//	printf("p");
	return 0;
}


static size_t headerCallback( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	Requester::DownloadData* dd = static_cast<Requester::DownloadData*>(userdata);
	if (dd->aborted)
		return 0;

//	printf("h");
	size_t realsize = size * nmemb;
	return realsize;
}


static void* threadFunc(void* ptr)
{
	Requester::DownloadData* dd = static_cast<Requester::DownloadData*>(ptr);

	CURL *curl;

//	printf("begin init\n");
	curl = curl_easy_init();
//	printf("end init\n");
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, dd->url.c_str());

		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, ptr);

		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, ptr);

		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, ptr);

//		printf("begin perform\n");
		dd->res = curl_easy_perform(curl);
//		printf("\nend perform: %d\n", dd->res);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}

//	printf("finished.\n");

	dd->finished = true;

	return NULL;
}

int req_load(const char* url, req_callback_type callback, void* callbackdata)
{
	Requester::DownloadData* dd = new Requester::DownloadData(url, callback, callbackdata);
	g_requester.activedata[g_requester.downloadId] = dd;

	pthread_create(&dd->thread, NULL, threadFunc, dd);

	return g_requester.downloadId++;
}

void req_tick()
{
	std::map<int, Requester::DownloadData*>::iterator iter;

	// delete from finished
	for (iter = g_requester.finisheddata.begin(); iter != g_requester.finisheddata.end();)
	{
		if (iter->second->finished == true && iter->second->tobedeleted == true)
		{
			delete iter->second;
			g_requester.finisheddata.erase(iter++);
		}
		else
			++iter;
	}

	// delete from active
	for (iter = g_requester.activedata.begin(); iter != g_requester.activedata.end();)
	{
		if (iter->second->finished == true && iter->second->tobedeleted == true)
		{
			delete iter->second;
			g_requester.activedata.erase(iter++);
		}
		else
			++iter;
	}


	std::vector<std::pair<int, Requester::DownloadData*> > callbacks;

	// move from active to finished
	for (iter = g_requester.activedata.begin(); iter != g_requester.activedata.end();)
	{
		if (iter->second->finished == true)
		{
			if (iter->second->aborted == false)
				if (iter->second->callback)
					callbacks.push_back(std::make_pair(iter->first, iter->second));

			g_requester.finisheddata[iter->first] = iter->second;

			g_requester.activedata.erase(iter++);
		}
		else
			++iter;
	}

	for (std::size_t i = 0; i < callbacks.size(); ++i)
	{
		int id = callbacks[i].first;
		Requester::DownloadData* dd = callbacks[i].second;
		dd->callback(id, dd->res, &dd->data[0], dd->data.size(), dd->callbackdata);
	}
}


void req_cancel(int id)
{
	std::map<int, Requester::DownloadData*>::iterator iter;

	iter = g_requester.activedata.find(id);
	if (iter != g_requester.activedata.end())
		iter->second->aborted = true;

	iter = g_requester.finisheddata.find(id);
	if (iter != g_requester.finisheddata.end())
		iter->second->aborted = true;
}

static void abort_join_delete()
{
	std::map<int, Requester::DownloadData*>::iterator iter;

	for (iter = g_requester.activedata.begin(); iter != g_requester.activedata.end(); ++iter)
		iter->second->aborted = true;

	for (iter = g_requester.activedata.begin(); iter != g_requester.activedata.end(); ++iter)
		pthread_join(iter->second->thread, NULL);

	for (iter = g_requester.activedata.begin(); iter != g_requester.activedata.end(); ++iter)
		delete iter->second;

	for (iter = g_requester.finisheddata.begin(); iter != g_requester.finisheddata.end(); ++iter)
		delete iter->second;

	g_requester.activedata.clear();
	g_requester.finisheddata.clear();
}

void req_delete(int id)
{
	std::map<int, Requester::DownloadData*>::iterator iter;

	iter = g_requester.activedata.find(id);
	if (iter != g_requester.activedata.end())
	{
		iter->second->aborted = true;
		iter->second->tobedeleted = true;
	}

	iter = g_requester.finisheddata.find(id);
	if (iter != g_requester.finisheddata.end())
	{
		iter->second->aborted = true;
		iter->second->tobedeleted = true;
	}
}

void req_init()
{
	g_requester.downloadId = 0;
}

void req_clean()
{
	abort_join_delete();
}
