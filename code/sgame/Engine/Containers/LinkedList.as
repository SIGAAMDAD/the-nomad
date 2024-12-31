
class LinkedListElement {
	LinkedListElement( ref@ data ) {
		@m_Data = @data;
	}
	LinkedListElement() {
	}

	const ref@ opImplConv() const {
		return @m_Data;
	}
	ref@ opImplConv() {
		return @m_Data;
	}

	const ref@ GetData() const {
		return @m_Data;
	}
	ref@ GetData() {
		return @m_Data;
	}

	private ref@ m_Data = null;
	LinkedListElement@ Next = null;
	LinkedListElement@ Prev = null;
};

class LinkedList {
	LinkedList() {
		@m_List.Prev =
		@m_List.Next =
			@m_List;
	}

	void Add( ref@ data ) {
		m_Data.Add( @data );
			
		LinkedListElement@ elem = @m_Data[ m_Data.Count() - 1 ];
		@elem.Prev = @m_List.Prev;
		@elem.Next = @m_List;

		@m_List.Prev.Next = @elem;
		@m_List.Prev = @elem;
	}
	uint Count() const {
		return m_Data.Count();
	}
	void Clear() {
		@m_List.Prev =
		@m_List.Next =
			@m_List;
			
		m_Data.Clear();
	}

	void LinkFirst( ref@ data ) {
		m_Data.Add( @data );

		LinkedListElement@ elem = @m_Data[ m_Data.Count() - 1 ];
		@elem.Prev = @m_List;
		@elem.Next = @m_List.Next;

		@m_List.Next.Prev = @elem;
		@m_List.Next = @elem;
	}

	LinkedListElement@ GetFirst() {
		return @m_List.Next;
	}
	LinkedListElement@ GetLast() {
		return @m_List.Prev;
	}

	private LinkedListElement m_List( null );
	private array<LinkedListElement> m_Data;
};