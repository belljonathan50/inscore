///<reference path="../controller/THandlersPrototypes.ts"/>
///<reference path="../lib/OSCAddress.ts"/>
///<reference path="IObject.ts"/>
///<reference path="IProxy.ts"/>

class IAppl extends IObject {
 
    protected kApplType: string;
    protected fReceivedMsgs: number;
    protected fRate: number;
    
    constructor() {
        super('ITL');
        this.kApplType = 'IAppl';
        this.fTypeString = 'IAppl';
        this.fReceivedMsgs = 0;
        this.fRate = 1000;
    } 

    getAppl() : IObject					{ return this; }
    getSet(): IMessage					{ let msg : IMessage; return msg; }
    getOSCAddress(): string 			{ return "/ITL"; }
    getRate(): number 					{ return this.fRate; }
    setRate(rate: number): void 		{ this.fRate = rate; }

	// prevent the output of color, position and time data at application level
	colorAble(): void		{}	
	positionAble(): void	{}
	timeAble() : void		{}

    processMsg (address: string, addressTail: string , msg: IMessage): msgStatus {
    	this.fReceivedMsgs++;

		let status = msgStatus.kBadAddress;
		let head = address;
		let tail = addressTail;

/*
		SIMessage msg = IMessage::create (*imsg);
		TAliasesMap::const_iterator i = fAliases.find(imsg->address());
		if (i != fAliases.end()) {
			msg->setAddress (i->second.first);
			if (i->second.second.size()) 
				msg->setMessage(i->second.second);
			head = OSCAddress.addressFirst(i->second.first);
			tail = OSCAddress.addressTail(i->second.first);
		}
*/
		if (tail.length) 			// application is not the final destination of the message
			status = super.processMsg(head, tail, msg);
	
//		else if (match(head)) {		// the message is for the application itself
		else if (head === "ITL") {		// the message is for the application itself
			status = this.execute(msg);
			if (status & msgStatus.kProcessed)
				this.setState(objState.kModified);
		}
//		if ((status == msgStatus.kProcessed) || (status == msgStatus.kProcessedNoChange))
			return status;
    }

	protected newObj (msg: IMessage, name: string): { status: msgStatus, obj?: IObject } 
    				{ return this.proxy_create(msg, name, this); }                
    protected proxy_create (msg: IMessage, name: string, parent: IObject): { status: msgStatus, obj?: IObject }	
    				{ return IProxy.execute (msg, name, parent); }
}