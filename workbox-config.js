module.exports = {
	globDirectory: './',
	globPatterns: [
		'*.{wasm,html,png,ico,json,js}'
	],
	swDest: 'sw.js',
	ignoreURLParametersMatching: [
		/^utm_/,
		/^fbclid$/
	],
	navigateFallback: '/ucblogo-code/ucblogo.html',
	runtimeCaching: [
		{
			handler: 'NetworkFirst',
			urlPattern: '*'
		}
	]
};