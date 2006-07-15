#include "imeventqueue.h"
#include "imevent.h"

#include <wx/wx.h>

using namespace std;

ImEventQueue::ImEventQueue() : crit_(new wxCriticalSection)
{;}

void
ImEventQueue::AddSendEvent(ImEvent* event)
{
	wxCriticalSectionLocker locker(*crit_);

	queue_.push_back(event);
}

ImEvent*
ImEventQueue::PopSendEvent()
{
	wxCriticalSectionLocker locker(*crit_);

	ImEvent* answer = (ImEvent*)0;
	if (queue_.size() > 0)
	{
		answer = queue_[queue_.size()-1];
		queue_.pop_back();
	}
	return answer;
}

void
ImEventQueue::ClearSendQueue()
{
	wxCriticalSectionLocker locker(*crit_);

	queue_.clear();
}

ImEventQueue::~ImEventQueue()
{
	delete crit_;
}


