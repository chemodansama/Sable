#include "gtest/gtest.h"

#include "Event.h"

using namespace sable;

TEST(Event, InvokeOnce)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	auto myListener = myEvent.attach(lambda);

	myEvent();

	EXPECT_EQ(invocationCount, 1);
}

TEST(Event, Invoke10)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	auto myListener = myEvent.attach(lambda);

	int n = 10;
	for (int i = 0; i < n; i++) {
		myEvent();
	}

	EXPECT_EQ(invocationCount, n);
}

TEST(Event, MoveListener)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	auto myListener = myEvent.attach(lambda);

	auto anotherListener = std::move(myListener);

	int n = 10;
	for (int i = 0; i < n; i++) {
		myEvent();
	}

	EXPECT_EQ(invocationCount, n);
}

TEST(Event, MoveListenerAndDestroyPrevious)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	Event<>::ListenerPtr anotherListener;
	{
		auto myListener = myEvent.attach(lambda);
		anotherListener = std::move(myListener);
	}

	int n = 10;
	for (int i = 0; i < n; i++) {
		myEvent();
	}

	EXPECT_EQ(invocationCount, n);
}

TEST(Event, DetachListener)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	auto myListener = myEvent.attach(lambda);
	myListener->detach();

	int n = 10;
	for (int i = 0; i < n; i++) {
		myEvent();
	}

	EXPECT_EQ(invocationCount, 0);
}

TEST(Event, MoveEvent)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	auto myListener = myEvent.attach(lambda);

	auto anotherEvent = std::move(myEvent);

	int n = 10;
	for (int i = 0; i < n; i++) {
		myEvent();
	}

	anotherEvent();

	EXPECT_EQ(invocationCount, 1);
}

TEST(Event, CopyEvent)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	auto myListener = myEvent.attach(lambda);

	auto eventCopy = myEvent;

	int n = 10;
	for (int i = 0; i < n; i++) {
		myEvent();
	}
	for (int i = 0; i < n; i++) {
		eventCopy();
	}

	EXPECT_EQ(invocationCount, 2 * n);
}

TEST(Event, MoveEventAndDetachListener)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	Event<> anotherEvent;
	{
		auto myListener = myEvent.attach(lambda);
		anotherEvent = std::move(myEvent);
	}

	EXPECT_EQ(anotherEvent.getListenersCount(), 0);
}

TEST(Event, AutomaticallyDetachedListener)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	myEvent.attach(lambda);

	myEvent();

	EXPECT_EQ(myEvent.getListenersCount(), 0);
	EXPECT_EQ(invocationCount, 0);
}

TEST(Event, InstanceMethodListener)
{
	Event<> myEvent;

	struct Callback
	{
		void callback()
		{
			invocationCount += 1;
		}

		int invocationCount = 0;
	};

	Callback callback;
	auto listener = myEvent.attach(&Callback::callback, &callback);

	myEvent();

	EXPECT_EQ(myEvent.getListenersCount(), 1);
	EXPECT_EQ(callback.invocationCount, 1);
}

TEST(Event, FunctorListener)
{
	Event<> myEvent;

	int invocationCount = 0;
	struct Callback
	{
		Callback(int* invocationCount)
			: invocationCount{ invocationCount }
		{
		}

		void operator()()
		{
			*invocationCount += 1;
		}

		int* invocationCount;
	};

	Callback callback{ &invocationCount };
	auto listener = myEvent.attach(callback);

	myEvent();

	EXPECT_EQ(myEvent.getListenersCount(), 1);
	EXPECT_EQ(invocationCount, 1);
}

TEST(Event, Parameters)
{
	Event<int, std::string> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount](int i, const std::string &s)
	{
		EXPECT_EQ(i, 1337);
		EXPECT_EQ(s, "hello world");
		invocationCount += 1;
	};

	auto myListener = myEvent.attach(lambda);

	myEvent(1337, "hello world");

	EXPECT_EQ(invocationCount, 1);
}

TEST(Event, MoveIntoEventWithListeners)
{
	Event<> myEvent;

	int invocationCount = 0;
	auto lambda = [&invocationCount]()
	{
		invocationCount += 1;
	};

	Event<> anotherEvent;
	{
		auto myListener = anotherEvent.attach(lambda);
		anotherEvent = std::move(myEvent);

		anotherEvent();

		EXPECT_EQ(invocationCount, 0);

		// myListener is destroyed here, shall see that original anotherEvent is destroyed.
	}
}