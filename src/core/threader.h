#pragma once

class CThread
{
public:
	template<typename Function, typename... Args>
	CThread(Function&& func, Args&&... args) : isDetached(false)
	{
		thread = std::thread(std::forward<Function>(func), std::forward<Args>(args)...);
	};
	~CThread()
	{
		// I think
		assert(!IsJoinable() || IsDetached());
	};

	inline void Detach() { thread.detach(); isDetached = true; }
	inline const bool IsDetached() const { return isDetached; }

	inline const bool IsJoinable() const { return thread.joinable(); }
	inline const bool Join() // return true or false depending if we joined correctly
	{
		if (IsJoinable())
		{
			thread.join();
			return true;
		}

		return false;
	}

	inline const std::thread::id GetID() const { return thread.get_id(); }

private:
	std::thread thread;
	bool isDetached;
};

class CThreadManager
{

};