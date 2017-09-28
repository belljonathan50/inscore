var verovio = verovio || {};

/***************************************************************************************************************************/
// Proxy the exported c++ methods
verovio.vrvToolkit = verovio.vrvToolkit || {};

// Constructor and destructor
// Toolkit *constructor()
verovio.vrvToolkit.constructor = Module.cwrap('vrvToolkit_constructor', 'number', []);

// void destructor(Toolkit *ic)
verovio.vrvToolkit.destructor = Module.cwrap('vrvToolkit_destructor', null, ['number']);

// bool edit(Toolkit *ic, const char *editorAction) 
verovio.vrvToolkit.edit = Module.cwrap('vrvToolkit_edit', 'number', ['number', 'string']);

// char *getElementsAtTime(Toolkit *ic, int time )
verovio.vrvToolkit.getElementsAtTime = Module.cwrap('vrvToolkit_getElementsAtTime', 'string', ['number', 'number']);

// char *getElementAttr(Toolkit *ic, const char *xmlId )
verovio.vrvToolkit.getElementAttr = Module.cwrap('vrvToolkit_getElementAttr', 'string', ['number', 'string']);

// char *getLog(Toolkit *ic)
verovio.vrvToolkit.getLog = Module.cwrap('vrvToolkit_getLog', 'string', ['number']);

// char *getMEI(Toolkit *ic, int pageNo, int scoreBased )
verovio.vrvToolkit.getMEI = Module.cwrap('vrvToolkit_getMEI', 'string', ['number', 'number', 'number']);

// char *getHumdrum(Toolkit *ic)
verovio.vrvToolkit.getHumdrum = Module.cwrap('vrvToolkit_getHumdrum', 'string');

// int getPageCount(Toolkit *ic)
verovio.vrvToolkit.getPageCount = Module.cwrap('vrvToolkit_getPageCount', 'number', ['number']);

// int getPageWithElement(Toolkit *ic, const char *xmlId)
verovio.vrvToolkit.getPageWithElement = Module.cwrap('vrvToolkit_getPageWithElement', 'number', ['number', 'string']);

// double getTimeForElement(Toolkit *ic, const char *xmlId)
verovio.vrvToolkit.getTimeForElement = Module.cwrap('vrvToolkit_getTimeForElement', 'number', ['number', 'string']);

// char *getVersion(Toolkit *ic)
verovio.vrvToolkit.getVersion = Module.cwrap('vrvToolkit_getVersion', 'string', ['number']);

// bool loadData(Toolkit *ic, const char *data )
verovio.vrvToolkit.loadData = Module.cwrap('vrvToolkit_loadData', 'number', ['number', 'string']);

// void redoLayout(Toolkit *ic)
verovio.vrvToolkit.redoLayout = Module.cwrap('vrvToolkit_redoLayout', null, ['number']);

// char *renderData(Toolkit *ic, const char *data, const char *options )
verovio.vrvToolkit.renderData = Module.cwrap('vrvToolkit_renderData', 'string', ['number', 'string', 'string']);

// char *renderPage(Toolkit *ic, int pageNo, const char *rendering_options )
verovio.vrvToolkit.renderPage = Module.cwrap('vrvToolkit_renderPage', 'string', ['number', 'number', 'string']);

// char *renderToMidi(Toolkit *ic, const char *rendering_options )
verovio.vrvToolkit.renderToMidi = Module.cwrap('vrvToolkit_renderToMidi', 'string', ['number', 'string']);

// void setOptions(Toolkit *ic, const char *options) 
verovio.vrvToolkit.setOptions = Module.cwrap('vrvToolkit_setOptions', null, ['number', 'string']);

// A pointer to the object - only one instance can be created for now
verovio.ptr = 0;

/***************************************************************************************************************************/

verovio.toolkit = function() {
	// check if we already have one instance
	if (verovio.ptr !== 0) {
		console.warn("For now only one instance of the toolkit can be created");
		this.ptr = verovio.ptr;
		return;
	}
	// if not, then create it
	this.ptr = verovio.vrvToolkit.constructor();
	verovio.ptr = this.ptr;
}

verovio.toolkit.prototype.destroy = function () {
	verovio.vrvToolkit.destructor(this.ptr);
	verovio.ptr = 0;
};

verovio.toolkit.prototype.edit = function (editorAction) {
	if (typeof editorAction === 'string') {
		console.warn("DEPRECATION WARNING: Passing a String to edit will be removed in next version of Verovio. Pass a JSON Object instead.");
		return verovio.vrvToolkit.edit(this.ptr, editorAction);
	}
	else {
		return verovio.vrvToolkit.edit(this.ptr, JSON.stringify(editorAction));
	}
	
};
