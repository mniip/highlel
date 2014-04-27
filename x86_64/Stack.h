template <typename T> inline void stackPush(Registers &r, Pager *p, T value)
{
	r.SP.rx += sizeof(T);
	p->store<T>(r.SP.rx, value);
}

template <typename T> inline T stackPop(Registers &r, Pager *p)
{
	T value = p->fetch<T>(r.SP.rx);
	r.SP.rx -= sizeof(T);
	return value;
}
