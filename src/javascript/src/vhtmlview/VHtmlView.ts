///<reference path="../model/IObject.ts"/>
///<reference path="../view/VObjectView.ts"/>

class VHtmlView extends VObjectView {
	protected fHtmlElt: HTMLElement;
	protected fParent: VHtmlView;
	protected fPoss: Array<Array<number>>;

	fTop: number; fLeft: number;
	fWidth: number; fHeight: number;

	constructor(elt: HTMLElement, parent?: VHtmlView) {
		super();
		this.fParent = parent;
		this.setPos(0, 0, 100, 100);
		this.fHtmlElt = elt;
		if (parent) parent.getHtml().appendChild(elt);
		this.setDefaultPositionStyle();
		this.fPoss = [];
	}

	setDefaultPositionStyle(): void { this.fHtmlElt.style.position = "absolute"; }
	getParent(): VObjectView { return this.fParent; }
	getHtml(): HTMLElement { return this.fHtmlElt; }
	remove(): void { this.fHtmlElt.parentNode.removeChild(this.fHtmlElt); }

	updateView(obj: IObject): void {
		this.updatePos(obj);
		this.updateColor(obj);
		this.updatePenControl(obj);
		this.updateEffects(obj);
		//test
		this.eventManager(obj, "mouseenter");
	}

	//------------------------------------------------------------------------------------
	// update color
	// target of color style depend on the html element implementation 
	colorTarget(): HTMLElement { return this.fHtmlElt; }
	updateColor(obj: IObject): void {
		if (obj.fColor.modified()) {
			let elt = this.colorTarget();
			elt.style.color = obj.fColor.getRGBString();
			let alpha = obj.fColor.getA() / 255;
			elt.style.opacity = alpha.toString();
		}
	}

	//------------------------------------------------------------------------------------
	// update pen
	updatePenControl(obj: IObject) {
		let penWidth = obj.fPenControl.getPenWidth();
		let penColor = obj.fPenControl.getPenColor().getRGBString();
		let penStyle = VHtmlView.penStyle2Dash(obj.fPenControl.getPenStyleNum());
		let penAlpha = obj.fPenControl.getPenColor().getA();

		let elt = this.getHtml();
		elt.style.borderWidth = penWidth + 'px';
		elt.style.borderColor = penColor;
		elt.style.borderStyle = penStyle;
	}

	//------------------------------------------------------------------------------------
	// update position
	getSize(obj: IObject): { w: number, h: number } {
		let size = obj.getSize();
		let strokeWidth = obj.fPenControl.getPenWidth();
		let w = this.relative2SceneWidth(size.w) + strokeWidth;
		let h = this.relative2SceneHeight(size.h) + strokeWidth;
		return { w: (w ? w : 1), h: (h ? h : 1) };
	}

	updatePos(obj: IObject): void {
		let pos = obj.getPosition();
		let size = this.getSize(obj);
		let z = obj.fPosition.getZOrder();
		let left = this.relative2SceneX(pos.x);
		let top = this.relative2SceneY(pos.y);
		this.setPos(top, left, size.w, size.h);		// store the metrics
		let shear = obj.fPosition.getShear();

		let elt = this.getHtml();
		elt.style.width = size.w + "px";
		elt.style.height = size.h + "px";
		elt.style.left = left + "px";
		elt.style.top = top + "px";
		elt.style.zIndex = z.toString();
		elt.style.transform = this.getTransform(obj) + " skewX(" + shear[0] + "rad) skewY(" + shear[1] + "rad)";
		elt.style.visibility = obj.fPosition.getVisible() ? "inherit" : "hidden";
	}

	// to be used for elts with auto width and height
	updateObjectSize(obj: IObject): void {
		let w = this.scene2RelativeWidth(this.fHtmlElt.clientWidth);
		let h = this.scene2RelativeHeight(this.fHtmlElt.clientHeight);
		obj.fPosition.setWidth(w);
		obj.fPosition.setHeight(h);
	}

	getParentsScale(obj: IObject): number {
		let p = obj.getParent();
		if (p) return p.fPosition.getScale() * this.getParentsScale(p);
		return 1;
	}
	getViewScale(obj: IObject): number { return obj.fPosition.getScale() * this.getParentsScale(obj); }

	getScale(obj: IObject): string {
		let scale = this.getViewScale(obj);
		return (scale == 1) ? "" : `scale(${scale},${scale}) `;
	}

	getTranslate(obj: IObject): string {
		let scale = obj.fPosition.getScale();
		let xo = obj.fPosition.getXOrigin() * scale;
		let yo = obj.fPosition.getYOrigin() * scale;
		let tx = -this.fWidth * (1 + xo) / 2.0;
		let ty = -this.fHeight * (1 + yo) / 2.0;
		return (tx || ty) ? `translate(${tx}px,${ty}px) ` : " ";
	}


	getRotateX(obj: IObject): string {
		let rotate = obj.getRotate();
		return (rotate.x) ? "rotateX(" + rotate.x + "deg)" : "";
	}

	getRotateY(obj: IObject): string {
		let rotate = obj.getRotate();
		return (rotate.y) ? "rotateY(" + rotate.y + "deg)" : "";
	}

	getRotateZ(obj: IObject): string {
		let rotate = obj.getRotate();
		return (rotate.z) ? `rotateZ(${rotate.z}deg) ` : "";
	}

	getTransform(obj: IObject): string {
		return this.getTranslate(obj) + this.getScale(obj) + this.getRotateX(obj) + this.getRotateY(obj) + this.getRotateZ(obj);
	}

	setPos(top: number, left: number, width: number, height: number): void {
		this.fTop = top;
		this.fLeft = left;
		this.fWidth = width;
		this.fHeight = height;
	}

	//------------------------------------------------------------------------------------
	// update effects
	updateEffects(obj: IObject): void {
		if (!obj.fEffect.fEffectModified) return;

		let elt = this.getHtml();
		switch (obj.fEffect.type()) {
			case eEffect.kNone: this.setNone();
				break;
			case eEffect.kBlur: this.setBlur(obj.fEffect.param(0));
				break;
			case eEffect.kShadow: this.setShadow(obj.fEffect.params());
				break;
		}
	}

	setNone(): void { this.setBlur(0); }
	setBlur(val: number): void {
		this.getHtml().style.boxShadow = "(0px 0px)";
		this.getHtml().style.filter = "blur( " + val + "px)";
	}

	setShadow(params: Array<number>): void {
		let color = new IColor(params.slice(2, 6));
		this.getHtml().style.boxShadow = color.getCSSRGBAString() + params[0] + "px " + params[1] + "px " + params[6] + "px";
		this.getHtml().style.filter = "blur(0)";
	}

	static penStyle2Dash(style: number): string {
		switch (style) {
			case 0: return "solid";
			case 1: return "dashed";
			case 2: return "dotted";
			default: return "solid";
		}
	}

	positionString(): string { return `top: ${this.fTop} left: ${this.fLeft} w: ${this.fWidth} h: ${this.fHeight}`; }

	// Maps the IObject [-1,1] y coordinate to the referenceRect().
	relative2SceneY(y: number): number { return this.fParent.fHeight * (y + 1.0) / 2.0; }

	// Maps the IObject [-1,1] x coordinate to the referenceRect().
	relative2SceneX(x: number): number { return this.fParent.fWidth * (x + 1.0) / 2.0; }

	// Maps the IObject [0,2] width value to the corresponding referenceRect() value.
	relative2SceneWidth(width: number): number { return Math.min(this.fParent.fWidth, this.fParent.fHeight) * width / 2.0; }

	// Maps the IObject [0,2] height value to the corresponding referenceRect() value.
	relative2SceneHeight(height: number): number { return Math.min(this.fParent.fWidth, this.fParent.fHeight) * height / 2.0; }

	// Maps the referenceRect() width value to the corresponding [0,2] value.
	scene2RelativeWidth(width: number): number { return (width * 2.0) / this.fParent.fWidth; }

	// Maps the referenceRect() height value to the corresponding [0,2] value.
	scene2RelativeHeight(height: number): number { return (height * 2.0) / this.fParent.fHeight; }

	// Maps the referenceRect() x value to the corresponding [-1,1] value.
	scene2RelativeX(x: number): number { return x / (this.fParent.fWidth / 2.0) - 1; }

	// Maps the referenceRect() y value to the corresponding [-1,1] value.
	scene2RelativeY(y: number): number { return y / (this.fParent.fHeight / 2.0) - 1; }

	// updates the local mapping (do nothing at IObject level) 
	//	updateLocalMapping (obj: IObject ): void;

	// initialize an object view (do nothing at IObject level)
	initialize(obj: IObject): void { }


	//---------------Events-----------------------
	// Manage and set or delete event listener
	eventManager(obj: IObject, eventName: string) : void{
		// Create Event on Object
		this.getHtml().addEventListener(eventName, (event) => {
			this.eventAction(event || window.event);
		});
	}

	// send datas to model
	eventAction(ev: any): void {
		ev = ev || window.event;

		// get position's datas
		let pageCoord = this.getPxCoord(ev);
		let parentCoord = this.getParentRelativeCoord(ev);
		let sceneCoord = this.getSceneRelativeCoord(ev);

		this.fPoss = [pageCoord, parentCoord, sceneCoord];
		this.sendPositions(this.fPoss);
	}

	//get event coordinate in px
	getPxCoord(ev : any): Array<number> {
		// get event position
		let x = ev.pageX;
		let y = ev.pageY;
		return [x, y];
	}

	// get relative to object event position
	getParentRelativeCoord(ev :any): Array<number> {
		let target = ev.target || ev.srcElement;
		// value in px
		let parentOffset = this.getParentsOffset(target, [target.offsetLeft, target.offsetTop])[1];
		let xp = ev.pageX - parentOffset[0];
		let yp = ev.pageY - parentOffset[1];

		// value : [-1,1]
		if (target.className != "inscore-scene"){
			xp = xp / (target.clientWidth / 2);
			yp = yp / (target.clientHeight / 2);
		}else {
			xp = xp / (target.clientWidth / 2) - 1;
			yp = yp / (target.clientHeight / 2) - 1;
		}
		return [xp, yp];
	}

		// get parent's offset
	getParentsOffset(elt: HTMLElement, xy : Array<number>):[HTMLElement, Array<number>] {
		let parent = elt.parentElement;
		if (parent){
			if (elt.className != "inscore-scene" && parent.className != "inscore-scene"){
				xy[0] += (parent.offsetLeft - (parent.clientWidth + 1)/2);
				xy[1] += (parent.offsetTop - (parent.clientHeight + 1)/2);
				return this.getParentsOffset(parent, xy);
			}else {
				xy[0] += (parent.offsetLeft);
				xy[1] += (parent.offsetTop);
				return [elt, xy];
			}
		} return [elt, xy];
	}

	// get relative to scene event position
	getSceneRelativeCoord(ev : any): Array<number> {
		let scene = this.getScene(this.getHtml());
		// value in px
		let xs = ev.pageX - (scene.offsetLeft + scene.parentElement.offsetLeft);
		let ys = ev.pageY - (scene.offsetTop  + scene.parentElement.offsetTop);
		// value : [-1,1]
		xs = xs / (scene.clientWidth / 2) - 1;
		ys = ys / (scene.clientHeight / 2) - 1;

		return [xs, ys];
	}

	getScene(elt: HTMLElement): HTMLElement {
		if (elt.className != "inscore-scene") return this.getScene(elt.parentElement);
		else return elt;
	}

	// Remove Target Event from an element
	removeHandler(event: string): void {
		this.getHtml().removeEventListener(event, this.eventAction);
	}

	sendPositions(poss : Array<Array<number>>): void {
	//	console.log("VHtmlView sendPositions poss : " + this.fPoss[1]);
	}
}