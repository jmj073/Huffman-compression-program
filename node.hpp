#ifndef NODE_H
#define NODE_H

template <typename T, int N>
struct PODNode
{
	T data;
	PODNode* links[N];

	T& get() {
		return data;
	}
	const T& get() const {
		return data;
	}

	void set(const T& _data) {
		data = _data;
	}
	void set(T&& _data) {
		data = _data;
	}

	PODNode* link(int idx) {
		return links[idx];
	 }

	const PODNode* link(int idx) const {
		return links[idx];
	}

	void set_link(int idx, PODNode* _link) {
		links[idx] = _link;
	}

	bool operator<(const PODNode& other) const
	{
		return data < other.data;
	}
	bool operator>(const PODNode& other) const
	{
		return data > other.data;
	}
	bool operator==(const PODNode& other) const
	{
		return data == other.data;
	}
	bool operator<=(const PODNode& other) const
	{
		return data <= other.data;
	}
	bool operator>=(const PODNode& other) const
	{
		return data >= other.data;
	}
	bool operator!=(const PODNode& other) const
	{
		return data != other.data;
	}
};

template <typename T>
void DestroyPODNodes(T* tree)
{
	if (!tree) return;
	for (T* node : tree->links)
		DestroyPODNodes(node);
	delete tree;
}

template <typename T>
class PODNodeGuard
{
public:
	explicit PODNodeGuard(T* node_ = nullptr)
		: node{ node_ } {}
	explicit PODNodeGuard(PODNodeGuard&& other)
		: node{ other.node } 
	{
		other.node = nullptr;
	}

	~PODNodeGuard() {
		DestroyPODNodes(node);
	}

	PODNodeGuard& operator=(const PODNodeGuard&) = delete;
	PODNodeGuard& operator=(PODNodeGuard&& other) {
		DestroyPODNodes(node);
		node = other.node;
		other.node = nullptr;
		return *this;
	}
	PODNodeGuard& operator=(T* node_) {
		DestroyPODNodes(node);
		node = node_;
		return *this;
	}

	T* get() {
		return node;
	}
	const T* get() const {
		return node;
	}

	T* operator->() {
		return node;
	}
	const T* operator->() const {
		return node;
	}

	T* release() {
		T* temp = node;
		node = nullptr;
		return temp;
	}

	operator bool() {
		return node;
	}

private:
	T* node;
};

#endif // NODE_H
