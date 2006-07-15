#ifndef YAWPSIM_NODE
#define YAWPSIM_NODE

#include <string>
#include <vector>
using namespace std;

#include <wx/string.h>

//#include <wx/event.h>
//#include <wx/datetime.h>

//class wxTimer;
//class wxTimerEvent;

class Node /*: public wxEvtHandler*/
{
private:
	//static int idCount;

public:
	Node(const wxString& nid_);

	/*Node& operator=(const Node& rhs)
	{
		id = rhs.id;
		nid = rhs.nid;
		hashes = rhs.hashes;
		next = rhs.next;
		previous = rhs.previous;
		next = rhs.next;
	}*/

	void Init(int maxHashes);

	void DoCrash();

	wxString Query(const wxString& hash);

	wxString Lookup(const wxString& hash);

	wxString BestStart(const wxString& hash);

	void Crash();

	~Node();

	bool IsSuccessor(wxString nid, wxString prev,wxString hash);

	void GenerateRandomHashes(int max);

	void RefreshFinger();

	void AddNodupToFinger(const wxString& nid);

	void Distribute();

	bool crash;
	wxString nid;
	vector<wxString> hashes;
	vector<wxString> finger;
	wxString next;
	wxString previous;
	//int id;

};

#endif
