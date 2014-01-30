#include <iostream>
#include <string>
#include<fstream>
#include <curl/curl.h>
#include<queue>
#include<list>
#include <algorithm>
using namespace std;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

list<string> done;
list<string> dont;
string getContents(string URL)
{
	CURL *curl;
  	CURLcode res;
  	string readBuffer;

  	curl = curl_easy_init();
  	if(curl) {
    	curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    	res = curl_easy_perform(curl);
    	curl_easy_cleanup(curl);
		return readBuffer;
	}
}
bool isRelative(string link)
{
	if(link.substr(0,4)!="http") return true;
	else return false;
}
bool isFile(string link,string format)
{
	if(link.substr(link.length()-format.length(),format.length())==format) return true;
	else return false;
}
void getFile(string contents,string link,string format)
{
		int j;
		for(j=link.length()-1;j>=0;j--) if(link[j]=='/') break;
		string name=format;
		name.append("/");
		name.append(link.substr(j+1,link.length()-1-j));
		ofstream output;
		output.open(name.c_str());
		output<<contents;
		output.close();
		cout<<name.substr(4,name.length()-4)<<" downloaded"<<endl;
}
string getDomain(string URL)
{
	int index,length;
	index=URL.find(".com");
	length=4;
	if(index==-1) {index=URL.find(".org");length=4;}
	if(index==-1) {index=URL.find(".net");length=4;}
	if(index==-1) {index=URL.find(".ac.in");length=6;}
	string domain=URL.substr(0,index+length);
	return domain;
}
void extractLinks(string contents,string URL,string format)
{
	int t=3000;
	int next=0;
	queue<string> q;
	while(t--)
	{
		int i = contents.find("href",next), end=contents.find(">",i);
		if(i==-1||end==-1) break;
		int r=0;	
		for(int j=i;j<end;j++)
		{
			if(contents[j]=='"'&&r==1) {end=j;break;}
			if(contents[j]=='"') r++;
		}
		next=end;
		string link;
		if(contents[i+5]=='"'||contents[i+5]=='\'') link=contents.substr(i+6,end-i-6);
		else link=contents.substr(i+5,end-i-5);
		if(isRelative(link))
		{
			if(link[0]=='/')  link.insert(0,getDomain(URL));
			else
			{
				int j,val;
				for(j=7;j<URL.length();j++)
					{
						if(URL[j]=='/'&&URL[j-1]!='/') val=j;
					}
				link.insert(0,string(URL.substr(0,val+1)));
			}
		}
		cout<<link<<endl;
		if(isFile(link,format))
		{
			string contents=getContents(link);
			getFile(contents,link,format);
		}
		else if(getDomain(link)==getDomain(URL)&&link!=URL&&link!=getDomain(URL))
		{
			bool push=true;
			list<string>::const_iterator iterator;
			for (iterator = done.begin(); iterator != done.end(); ++iterator) {
    			if(*iterator==link) push=false;
			}
			for (iterator = dont.begin(); iterator != dont.end(); ++iterator) {
    			if(link.find(*iterator)+1) push=false;
			}
			if(push&&link.find("https://archive.org/details/")+1){cout<<"pushing"<<link<<endl; q.push(link);}
		}
	}
	while(!q.empty())
	{
		string get=q.front();
		q.pop();
		done.push_back(get);
		cout<<"----------------------------"<<endl;
		cout<<"Entering to:>"<<get<<endl;
		cout<<"----------------------------"<<endl;
		string content=getContents(get);
		if(get!=getDomain(URL))extractLinks(content,get,format);
	}
}
int main()
{
	string URL="https://archive.org/search.php?query=fiction%20AND%20mediatype%3Atexts";
	string format="epub";
	//cout<<"Enter url"<<endl<<">>";
	//cin>>URL;
	//cout<<"Enter format"<<endl<<">>";
	//--------------------------------------------------
	//Links not to crawl
	dont.push_back("https://archive.org/details/movies");
	dont.push_back("https://archive.org/details/texts");
	dont.push_back("https://archive.org/details/audio");
	dont.push_back("https://archive.org/details/software");
	dont.push_back("https://archive.org/details/tv");
	dont.push_back("https://archive.org/details/#");
	string contents = getContents(URL);
	extractLinks(contents,URL,format);
	return 0;
}
