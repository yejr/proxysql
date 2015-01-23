#ifndef __CLASS_MYSQL_HOSTGROUPS_MANAGER_H
#define __CLASS_MYSQL_HOSTGROUPS_MANAGER_H
#include "proxysql.h"
#include "cpp.h"

#define MYHGM_MYSQL_SERVERS "CREATE TABLE mysql_servers ( hostgroup_id INT NOT NULL DEFAULT 0, hostname VARCHAR NOT NULL , port INT NOT NULL DEFAULT 3306, weight INT CHECK (weight >= 0) NOT NULL DEFAULT 1 , status INT CHECK (status IN (0, 1, 2, 3)) NOT NULL DEFAULT 0, mem_pointer INT NOT NULL DEFAULT 0, PRIMARY KEY (hostgroup_id, hostname, port) )"
#define MYHGM_MYSQL_SERVERS_INCOMING "CREATE TABLE mysql_servers_incoming ( hostgroup_id INT NOT NULL DEFAULT 0, hostname VARCHAR NOT NULL , port INT NOT NULL DEFAULT 3306, weight INT CHECK (weight >= 0) NOT NULL DEFAULT 1 , status INT CHECK (status IN (0, 1, 2, 3)) NOT NULL DEFAULT 0, PRIMARY KEY (hostgroup_id, hostname, port))"


class MySrvConnList;
class MySrvC;
class MySrvList;
class MyHGC;

enum MySerStatus {
	MYSQL_SERVER_STATUS_ONLINE,
	MYSQL_SERVER_STATUS_SHUNNED,
	MYSQL_SERVER_STATUS_OFFLINE_SOFT,
	MYSQL_SERVER_STATUS_OFFLINE_HARD
};



class MySrvConnList {
	private:
	MySrvC *mysrvc;
	PtrArray *conns;
	int find_idx(MySQL_Connection *);
	public:
	MySrvConnList(MySrvC *);
	~MySrvConnList();
	void add(MySQL_Connection *);
	void remove(MySQL_Connection *);
	MySQL_Connection * get_random_MyConn();
};

class MySrvC {	// MySQL Server Container
	public:
	MyHGC *myhgc;
	char *address;
	uint16_t port;
	uint16_t flags;
	unsigned int weight;
	enum MySerStatus status;
	MySrvConnList *ConnectionsUsed;
	MySrvConnList *ConnectionsFree;
	MySrvC(char *, uint16_t, unsigned int, enum MySerStatus);
	~MySrvC();
};

class MySrvList {	// MySQL Server List
	private:
	MyHGC *myhgc;
	PtrArray *servers;
	int find_idx(MySrvC *);
//	int find_idx(MySQL_Connection *);
	public:
	unsigned int cnt();
	MySrvList(MyHGC *);
	~MySrvList();
	void add(MySrvC *);
	void remove(MySrvC *);
	MySrvC * idx(unsigned int);
};

class MyHGC {	// MySQL Host Group Container
	public:
	unsigned int hid;
	MySrvList *mysrvs;
	MyHGC(int);
	~MyHGC();
	MySrvC *get_random_MySrvC();
};

class MySQL_HostGroups_Manager {
	private:
	SQLite3DB	*mydb;
	rwlock_t rwlock;
	PtrArray *MyHostGroups;

	MyHGC * MyHGC_find(unsigned int);
	MyHGC * MyHGC_create(unsigned int);

	public:
	MySQL_HostGroups_Manager();
	~MySQL_HostGroups_Manager();
	void rdlock();
	void rdunlock();
	void wrlock();
	void wrunlock();
	bool server_add(unsigned int hid, char *add, uint16_t p=3306, unsigned int _weight=1, enum MySerStatus status=MYSQL_SERVER_STATUS_ONLINE);
	bool commit();

	MyHGC * MyHGC_lookup(unsigned int);
	
	void MyConn_add_to_poll(MySQL_Connection *);

	MySQL_Connection * get_MyConn_from_poll(unsigned int);

	void push_MyConn_to_poll(MySQL_Connection *);
	void destroy_MyConn_from_poll(MySQL_Connection *);	
};

#endif /* __CLASS_MYSQL_HOSTGROUPS_MANAGER_H */