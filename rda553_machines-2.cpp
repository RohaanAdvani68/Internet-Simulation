#include <iostream>
#include <stdlib.h>
#include <string>

#include "machines.h"

//*****************************************//
// node class functions

node::node(string n, IPAddress a) {
    name = new string;
    *name = n;
    my_address = a;
}

node::~node() {
    delete name;
}

void node::display() {
    
    cout << "   Name: " << *name << "   IP address: ";
    my_address.display();
}

int node::amIThisComputer(IPAddress a) {
    if(my_address.sameAddress(a))
        return 1;
    else
        return 0;
}

int node::myType() {
    return 0;
}

IPAddress node::myAddress()
{
    return my_address;
}

//*****************************************//
// laptop class functions

laptop::laptop(string n, IPAddress a) : node(n,a)  {
    incoming = outgoing = NULL;
    my_server.parse("0.0.0.0");
}

int laptop::myType() {
    return LAPTOP;
}

void laptop::initiateDatagram(datagram* d) {
    outgoing = d;
}

void laptop::receiveDatagram(datagram* d) {
    incoming = d;
}

int  laptop::canAcceptConnection(int machine_type) {
    if(machine_type!=SERVER) return 0;
    return my_server.isNULL(); //we can only connect if the server is empty
}

void laptop::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER) my_server = x;
}

void laptop::transferDatagram() {
    int i;
    extern node* network[MAX_MACHINES];
    
    if(outgoing==NULL) return;
    if(my_server.isNULL()) return;
    for (i = 0; i < MAX_MACHINES; i++)
    {
        if (network[i] != NULL)
        {
            if (network[i]->amIThisComputer(my_server))
                break;
        }
    }
    network[i]->receiveDatagram(outgoing);
    outgoing = NULL;
}

void laptop::display() {
    
    cout << "Laptop computer:  ";
    node::display();
    
    if(my_server.isNULL()) {
        cout << "\n    Not connected to any server.\n";
    }
    else {
        cout << "\nConnected to ";
        my_server.display();
        cout << "\n";
    }
    
    cout << "\n   Incoming datagram:  ";
    if(incoming==NULL) cout << "No datagram.";
    else               {cout << "\n";  incoming->display(); }
    
    cout << "\n   Outgoing datagram:  ";
    if(outgoing==NULL) cout << "No datagram.";
    else               {cout << "\n"; outgoing->display(); }
    
}

void laptop::consumeDatagram()
{
    incoming=NULL;
}

int laptop::canReceiveDatagram()
{
    if(incoming==NULL)
        return 1;
    else
        return 0;
}

/**************new*************/
laptop::~laptop()
{
    if (incoming != NULL)
        delete incoming;
    if (outgoing != NULL)
        delete outgoing;
}
/**************new*************/

//*****************************************//
// server class functions

server::server(string n, IPAddress a) : node(n,a)  {
    number_of_laptops = number_of_wans = 0;
    dlist = new msg_list;
}


int server::myType() {
    return SERVER;
}

int  server::canAcceptConnection(int machine_type) {
    if(machine_type==LAPTOP)
        return (number_of_laptops<8);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    return 0;
}

void server::connect(IPAddress x, int machine_type) {
    if(machine_type==LAPTOP)
        laptop_list[number_of_laptops++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void server::receiveDatagram(datagram* d) {
    dlist->append(d);
}

void server::display() {
    int i;
    
    cout << "Server computer:  ";
    node::display();
    
    cout << "\n   Connections to laptops: ";
    if(number_of_laptops==0) cout << "    List is empty.";
    else for(i=0; i<number_of_laptops; i++) {
        cout << "\n      Laptop " << i+1 << ":   ";
        laptop_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

void server::transferDatagram()
{
    extern node* network[MAX_MACHINES];
    msg_list* tmp=new msg_list;
    datagram* d=dlist->returnFront();
    IPAddress ip1;
    while (d!=NULL)
    {
        ip1=d->destinationAddress();
        int dest_octad1=ip1.firstOctad();
        int server_octad1=my_address.firstOctad();
        if(dest_octad1==server_octad1)//if destination laptop lies in the servers LAN
        {
            //check if laptop is connected to the server in same LAN
            bool b=true;
            for(int j=0;j<number_of_laptops;j++)
            {
                if(laptop_list[j].sameAddress(ip1))
                {
                    b=false;
                    break;
                }
            }
            if(b)//IF SERVER IS NOT CONNECTED TO THE LAPTOP IN SAME LAN
            {
                tmp->append(d);
            }
            else
            {
                int i;
                for(i=0;i<MAX_MACHINES;i++)
                {
                    if(network[i]==NULL)
                        continue;
                    IPAddress ip2=network[i]->myAddress();
                    if(ip1.sameAddress(ip2)==1)//if they have the same IP
                    {
                        if(((laptop*)network[i])->canReceiveDatagram()==1)
                        {
                            network[i]->receiveDatagram(d);//sending datagram o destination laptop in within the server
                        }
                        else
                            tmp->append(d);
                        break;
                    }
                }
            }
        }
        else
        {
            //find WAN IP with lowest abs minimum diff
            if(number_of_wans==0)//if not connected to any WANs
            {
                tmp->append(d);
            }
            else
            {
                int oct1=WAN_list[0].firstOctad();
                int min=abs(dest_octad1-oct1);
                IPAddress minIP=WAN_list[0];
                for(int j=1;j<number_of_wans;j++)
                {
                    oct1=WAN_list[j].firstOctad();
                    if(abs(dest_octad1-oct1)<min)
                    {
                        min=abs(dest_octad1-oct1);
                        minIP=WAN_list[j];
                    }
                }
                int k;
                for (k=0;k<MAX_MACHINES;k++)//to find the WAN with the given IP in the network
                {
                    if((network[k]->myAddress()).sameAddress(minIP))//if they have same IP
                    {
                        break;
                    }
                }
                if(k==MAX_MACHINES)//if no machine with same IP found
                {
                    tmp->append(d);
                }
                else
                {
                    network[k]->receiveDatagram(d);
                }
            }
        }
        
        d=dlist->returnFront();
    }
    dlist->deleteList();
    dlist=tmp;
}

/**************new*************/
server::~server()
{
    dlist->deleteList();
}
/**************new*************/

//*****************************************//
// WAN class functions

WAN::WAN(string n, IPAddress a) : node(n,a)  {
    number_of_servers = number_of_wans = 0;
    dlist = new msg_list;
}

int WAN::myType() {
    return WAN_MACHINE;
}

int  WAN::canAcceptConnection(int machine_type) {
    if(machine_type==SERVER)
        return (number_of_servers<4);
    else if(machine_type==WAN_MACHINE)
        return (number_of_wans<4);
    
    return 0;
}

void WAN::connect(IPAddress x, int machine_type) {
    if(machine_type==SERVER)
        server_list[number_of_servers++] = x;
    else if(machine_type==WAN_MACHINE)
        WAN_list[number_of_wans++] = x;
}

void WAN::receiveDatagram(datagram* d) {
    dlist->append(d);
}


void WAN::display() {
    int i;
    
    cout << "WAN computer:  ";
    node::display();
    
    cout << "\n   Connections to servers: ";
    if(number_of_servers==0) cout << "    List is empty.";
    else for(i=0; i<number_of_servers; i++) {
        cout << "\n      Server " << i+1 << ":   ";
        server_list[i].display();
    }
    cout << "\n\n   Connections to WANs:    ";
    if(number_of_wans==0) cout << "    List is empty.";
    else for(i=0; i<number_of_wans; i++) {
        cout << "\n         WAN " << i+1 << ":   ";
        WAN_list[i].display();
    }
    
    cout << "\n\n   Message list:\n";
    dlist->display();
    
    cout << "\n\n";
    
}

void WAN::transferDatagram()
{
    extern node* network[MAX_MACHINES];
    msg_list* tmp=new msg_list;
    datagram* d=dlist->returnFront();
    IPAddress dest_ip;
    while (d!=NULL)
    {
        dest_ip=d->destinationAddress();
        int dest_octad1=dest_ip.firstOctad();
        bool b=true;
        for(int i=0;i<number_of_servers;i++)
        {
            int serv_oct1=server_list[i].firstOctad();
            if(serv_oct1==dest_octad1)//if server with same LAN ID is found
            {
                for(int j=0;j<MAX_MACHINES;j++)//finding the server in the network
                {
                    if(network[j]==NULL) continue;
                    if(network[j]->amIThisComputer(server_list[i])==1)
                    {
                        network[j]->receiveDatagram(d);
                        b=false;//sent to server
                        break;
                    }
                }
                break;
            }
        }
        if(b)//if destination_server is not in the WANs server list
        {
                //then we need to send it to the WAN in the WAN's WAN list that has min abs diff
            if(number_of_wans==0)
            {
                tmp->append(d);
            }
            else
            {
                IPAddress minIP=WAN_list[0];
                int min=abs(minIP.firstOctad()-dest_octad1);
                for(int i=0;i<number_of_wans;i++)
                {
                    if(abs(WAN_list[i].firstOctad()-dest_octad1)<min)
                    {
                        min=abs(WAN_list[i].firstOctad()-dest_octad1);
                        minIP=WAN_list[i];
                    }
                }
                for(int j=0;j<MAX_MACHINES;j++)//finding the WAN in the network
                {
                    if(network[j]==NULL) continue;
                    if(network[j]->amIThisComputer(minIP)==1)
                    {
                        network[j]->receiveDatagram(d);//transferring the datagram
                        break;
                    }
                }

            }
        }
        
        d=dlist->returnFront();
    }
    dlist->deleteList();
    dlist=tmp;
}

/**************new*************/
WAN::~WAN()
{
    dlist->deleteList();
}
/**************new*************/




















