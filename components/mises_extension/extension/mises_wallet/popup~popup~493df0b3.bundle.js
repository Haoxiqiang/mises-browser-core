(window.webpackJsonp=window.webpackJsonp||[]).push([[9],{1029:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0})},1030:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0})},1031:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0})},1032:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0})},1033:function(t,e,i){"use strict";var n=this&&this.__createBinding||(Object.create?function(t,e,i,n){void 0===n&&(n=i),Object.defineProperty(t,n,{enumerable:!0,get:function(){return e[i]}})}:function(t,e,i,n){t[n=void 0===n?i:n]=e[i]}),r=this&&this.__exportStar||function(t,e){for(var i in t)"default"===i||Object.prototype.hasOwnProperty.call(e,i)||n(e,t,i)};Object.defineProperty(e,"__esModule",{value:!0}),r(i(1034),e)},1034:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0})},1035:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0})},1036:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.EthSignType=void 0,(e=e.EthSignType||(e.EthSignType={})).MESSAGE="message",e.TRANSACTION="transaction",e.EIP712="eip-712"},1037:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0})},1038:function(t,e){},1039:function(t,e){},1137:function(t,e){},140:function(t,e,i){"use strict";var n=this&&this.__importDefault||function(t){return t&&t.__esModule?t:{default:t}};Object.defineProperty(e,"__esModule",{value:!0}),e.Uint=e.Int=void 0;const r=n(i(393)),o=i(141),s=i(395);class c{constructor(t){if("string"==typeof(t="number"==typeof t?t.toString():t)&&!s.isValidIntegerString(t)){if(!s.isExponentDecString(t))throw new Error("invalid integer: "+t);t=s.exponentDecStringToDecString(t)}this.int=r.default(t),this.checkBitLen()}checkBitLen(){if(this.int.abs().gt(c.maxInt))throw new Error("Integer out of range "+this.int.toString())}toString(){return this.int.toString(10)}isNegative(){return this.int.isNegative()}isPositive(){return this.int.isPositive()}isZero(){return this.int.eq(r.default(0))}equals(t){return this.int.equals(t.int)}gt(t){return this.int.gt(t.int)}gte(t){return this.int.greaterOrEquals(t.int)}lt(t){return this.int.lt(t.int)}lte(t){return this.int.lesserOrEquals(t.int)}abs(){return new c(this.int.abs())}absUInt(){return new a(this.int.abs())}add(t){return new c(this.int.add(t.int))}sub(t){return new c(this.int.subtract(t.int))}mul(t){return new c(this.int.multiply(t.int))}div(t){return new c(this.int.divide(t.int))}mod(t){return new c(this.int.mod(t.int))}neg(){return new c(this.int.negate())}pow(t){return new c(this.int.pow(t.toBigNumber()))}toDec(){return new o.Dec(this)}toBigNumber(){return this.int}}(e.Int=c).maxInt=r.default("115792089237316195423570985008687907853269984665640564039457584007913129639935");class a{constructor(t){if("string"==typeof(t="number"==typeof t?t.toString():t)&&!s.isValidIntegerString(t)){if(!s.isExponentDecString(t))throw new Error("invalid integer: "+t);t=s.exponentDecStringToDecString(t)}if(this.uint=r.default(t),this.uint.isNegative())throw new TypeError("Uint should not be negative");this.checkBitLen()}checkBitLen(){if(this.uint.abs().bitLength().gt(256))throw new Error("Integer out of range "+this.uint.toString())}toString(){return this.uint.toString(10)}isZero(){return this.uint.eq(r.default(0))}equals(t){return this.uint.equals(t.uint)}gt(t){return this.uint.gt(t.uint)}gte(t){return this.uint.greaterOrEquals(t.uint)}lt(t){return this.uint.lt(t.uint)}lte(t){return this.uint.lesserOrEquals(t.uint)}add(t){return new a(this.uint.add(t.uint))}sub(t){return new a(this.uint.subtract(t.uint))}mul(t){return new a(this.uint.multiply(t.uint))}div(t){return new a(this.uint.divide(t.uint))}mod(t){return new a(this.uint.mod(t.uint))}pow(t){return new a(this.uint.pow(t.toBigNumber()))}toDec(){return new o.Dec(new c(this.toString()))}toBigNumber(){return this.uint}}e.Uint=a},141:function(t,e,i){"use strict";var n=this&&this.__importDefault||function(t){return t&&t.__esModule?t:{default:t}};Object.defineProperty(e,"__esModule",{value:!0}),e.Dec=void 0;const o=n(i(393)),r=i(140),s=i(283),c=i(395);class a{constructor(t,e=0){if("string"==typeof(t="number"==typeof t?t.toString():t)){if(0===t.length)throw new Error("empty string");if(!c.isValidDecimalString(t)){if(!c.isExponentDecString(t))throw new Error("invalid decimal: "+t);t=c.exponentDecStringToDecString(t)}var i=a.reduceDecimalsFromString(t);i.isDownToZero,0<=(t=i.res).indexOf(".")&&(e=t.length-t.indexOf(".")-1,t=t.replace(".","")),this.int=o.default(t)}else t instanceof r.Int?this.int=o.default(t.toString()):this.int=o.default(t);this.int=this.int.multiply(a.calcPrecisionMultiplier(e)),this.checkBitLen()}static calcPrecisionMultiplier(t){if(t<0)throw new Error("Invalid prec");if(t>a.precision)throw new Error("Too much precision");var e;return a.precisionMultipliers[t.toString()]?a.precisionMultipliers[t.toString()]:(e=a.precision-t,e=o.default(10).pow(e),a.precisionMultipliers[t.toString()]=e)}static reduceDecimalsFromString(t){var e=t.indexOf(".");return e<0||(e=t.length-1-e-a.precision)<=0?{res:t,isDownToZero:!1}:{res:t=t.slice(0,t.length-e),isDownToZero:/^[0.]*$/.test(t)}}checkBitLen(){if(this.int.abs().gt(a.maxDec))throw new Error("Integer out of range "+this.int.toString())}isZero(){return this.int.eq(o.default(0))}isNegative(){return this.int.isNegative()}isPositive(){return this.int.isPositive()}equals(t){return this.int.eq(t.int)}gt(t){return this.int.gt(t.int)}gte(t){return this.int.geq(t.int)}lt(t){return this.int.lt(t.int)}lte(t){return this.int.leq(t.int)}neg(){return new a(this.int.negate(),a.precision)}abs(){return new a(this.int.abs(),a.precision)}add(t){return new a(this.int.add(t.int),a.precision)}sub(t){return new a(this.int.subtract(t.int),a.precision)}pow(e){if(e.isZero())return new a(1);if(e.isNegative())return new a(1).quo(this.pow(e.abs()));let i=new a(this.int,a.precision),n=new a(1);for(let t=e;t.gt(new r.Int(1));t=t.div(new r.Int(2)))t.mod(new r.Int(2)).isZero()||(n=n.mul(i)),i=i.mul(i);return i.mul(n)}mul(t){return new a(this.mulRaw(t).chopPrecisionAndRound(),a.precision)}mulTruncate(t){return new a(this.mulRaw(t).chopPrecisionAndTruncate(),a.precision)}mulRaw(t){return new a(this.int.multiply(t.int),a.precision)}quo(t){return new a(this.quoRaw(t).chopPrecisionAndRound(),a.precision)}quoTruncate(t){return new a(this.quoRaw(t).chopPrecisionAndTruncate(),a.precision)}quoRoundUp(t){return new a(this.quoRaw(t).chopPrecisionAndRoundUp(),a.precision)}quoRaw(t){var e=a.calcPrecisionMultiplier(0),e=this.int.multiply(e).multiply(e);return new a(e.divide(t.int),a.precision)}isInteger(){var t=a.calcPrecisionMultiplier(0);return this.int.remainder(t).equals(o.default(0))}chopPrecisionAndRound(){var t,e,i;return this.isNegative()?this.abs().chopPrecisionAndRound().negate():(t=(e=a.calcPrecisionMultiplier(0)).divide(o.default(2)),{quotient:e,remainder:i}=this.int.divmod(e),i.equals(o.default(0))||i.lt(t)||!i.gt(t)&&e.divide(o.default(2)).equals(o.default(0))?e:e.add(o.default(1)))}chopPrecisionAndRoundUp(){var t,e;return this.isNegative()?this.abs().chopPrecisionAndTruncate().negate():(t=a.calcPrecisionMultiplier(0),{quotient:t,remainder:e}=this.int.divmod(t),e.equals(o.default(0))?t:t.add(o.default(1)))}chopPrecisionAndTruncate(){var t=a.calcPrecisionMultiplier(0);return this.int.divide(t)}toString(t=a.precision,e=!1){var i=a.calcPrecisionMultiplier(0),{quotient:i,remainder:n}=this.int.abs().divmod(i);let r=n.toString(10);for(let t=0,e=r.length;t<a.precision-e;t++)r="0"+r;return r=r.substring(0,t),(this.isNegative()&&!(i.eq(o.default(0))&&0===r.length)?"-":"")+(e?s.CoinUtils.integerStringToUSLocaleString(i.toString()):i.toString())+(0<r.length?"."+r:"")}round(){return new r.Int(this.chopPrecisionAndRound())}roundUp(){return new r.Int(this.chopPrecisionAndRoundUp())}truncate(){return new r.Int(this.chopPrecisionAndTruncate())}roundDec(){return new a(this.chopPrecisionAndRound(),0)}roundUpDec(){return new a(this.chopPrecisionAndRoundUp(),0)}truncateDec(){return new a(this.chopPrecisionAndTruncate(),0)}}(e.Dec=a).precision=18,a.decimalPrecisionBits=60,a.maxDec=o.default("133499189745056880149688856635597007162669032647290798121690100488888732861290034376435130433535"),a.precisionMultipliers={}},1630:function(t,e){},175:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.DecUtils=void 0;const n=i(141),r=i(140);class o{static trim(t){let e="string"==typeof t?t:t.toString();if(!(e.indexOf(".")<0)){for(let t=e.length-1;0<=t&&"0"===e[t];t--)e=e.slice(0,t);0<e.length&&"."===e[e.length-1]&&(e=e.slice(0,e.length-1))}return e}static getTenExponentN(t){if(t<-n.Dec.precision)throw new Error("Too little precision");var e;return o.tenExponentNs[t.toString()]?o.tenExponentNs[t.toString()]:(e=new n.Dec(10).pow(new r.Int(t)),o.tenExponentNs[t.toString()]=e)}static getTenExponentNInPrecisionRange(t){if(t>n.Dec.precision)throw new Error("Too much precision");return o.getTenExponentN(t)}static getPrecisionDec(t){return o.getTenExponentNInPrecisionRange(t)}}(e.DecUtils=o).tenExponentNs={}},210:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.IntPretty=void 0;const n=i(140),r=i(141),o=i(175),s=i(283);e.IntPretty=class c{constructor(t){if(this.floatingDecimalPointRight=0,this._options={maxDecimals:0,trim:!1,shrink:!1,ready:!0,locale:!0,inequalitySymbol:!1,inequalitySymbolSeparator:" "},"object"==typeof t&&"toDec"in t?t=t.toDec():t instanceof r.Dec||(t=new r.Dec(t)),t.isZero())this.dec=t;else{let e=t,i=0;for(let t=0;t<18&&(e.truncate().equals(new n.Int(0))||!e.equals(new r.Dec(e.truncate())));t++)e=e.mul(new r.Dec(10)),i++;this.dec=t,this._options.maxDecimals=i}}get options(){return this._options}moveDecimalPointLeft(t){var e=this.clone();return e.floatingDecimalPointRight+=-t,e}moveDecimalPointRight(t){var e=this.clone();return e.floatingDecimalPointRight+=t,e}increasePrecision(t){return this.moveDecimalPointLeft(t)}decreasePrecision(t){return this.moveDecimalPointRight(t)}maxDecimals(t){var e=this.clone();return e._options.maxDecimals=t,e}inequalitySymbol(t){var e=this.clone();return e._options.inequalitySymbol=t,e}inequalitySymbolSeparator(t){var e=this.clone();return e._options.inequalitySymbolSeparator=t,e}trim(t){var e=this.clone();return e._options.trim=t,e}shrink(t){var e=this.clone();return e._options.shrink=t,e}locale(t){var e=this.clone();return e._options.locale=t,e}ready(t){var e=this.clone();return e._options.ready=t,e}get isReady(){return this._options.ready}add(t){return t instanceof r.Dec||(t=t.toDec()),(t=new c(this.toDec().add(t)))._options=Object.assign({},this._options),t}sub(t){return t instanceof r.Dec||(t=t.toDec()),(t=new c(this.toDec().sub(t)))._options=Object.assign({},this._options),t}mul(t){return t instanceof r.Dec||(t=t.toDec()),(t=new c(this.toDec().mul(t)))._options=Object.assign({},this._options),t}quo(t){return t instanceof r.Dec||(t=t.toDec()),(t=new c(this.toDec().quo(t)))._options=Object.assign({},this._options),t}toDec(){if(0===this.floatingDecimalPointRight)return this.dec;if(0<this.floatingDecimalPointRight)return this.dec.mulTruncate(o.DecUtils.getTenExponentN(this.floatingDecimalPointRight));{let t=-this.floatingDecimalPointRight,e=this.dec;for(;0<t;){if(!(t>=r.Dec.precision)){e=e.mulTruncate(o.DecUtils.getTenExponentN(-(t%r.Dec.precision)));break}e=e.mulTruncate(o.DecUtils.getTenExponentN(-r.Dec.precision)),t-=r.Dec.precision}return e}}toString(){return this.toStringWithSymbols("","")}toStringWithSymbols(t,e){var i=this.toDec();if(this._options.inequalitySymbol&&!i.isZero()&&i.abs().lt(o.DecUtils.getTenExponentN(-this._options.maxDecimals))){const r=i.isNegative();return(r?">":"<")+this._options.inequalitySymbolSeparator+(r?"-":"")+t+o.DecUtils.getTenExponentN(-this._options.maxDecimals).toString(this._options.maxDecimals,this._options.locale)+e}let n;n=this._options.shrink?s.CoinUtils.shrinkDecimals(i,0,this._options.maxDecimals,this._options.locale):i.toString(this._options.maxDecimals,this._options.locale);const r="-"===(n=this._options.trim?o.DecUtils.trim(n):n).charAt(0);return r&&(n=n.slice(1)),(r?"-":"")+t+n+e}clone(){var t=new c(this.dec);return t.dec=this.dec,t.floatingDecimalPointRight=this.floatingDecimalPointRight,t._options=Object.assign({},this._options),t}}},26:function(t,e,i){"use strict";var n=this&&this.__createBinding||(Object.create?function(t,e,i,n){void 0===n&&(n=i),Object.defineProperty(t,n,{enumerable:!0,get:function(){return e[i]}})}:function(t,e,i,n){t[n=void 0===n?i:n]=e[i]}),r=this&&this.__exportStar||function(t,e){for(var i in t)"default"===i||Object.prototype.hasOwnProperty.call(e,i)||n(e,t,i)};Object.defineProperty(e,"__esModule",{value:!0}),r(i(210),e),r(i(658),e),r(i(394),e),r(i(140),e),r(i(141),e),r(i(283),e),r(i(175),e),r(i(659),e),r(i(660),e)},283:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.CoinUtils=void 0;const s=i(394),n=i(140),c=i(141),r=i(175);class a{static createCoinsFromPrimitives(t){return t.map(t=>new s.Coin(t.denom,t.amount))}static amountOf(t,e){t=t.find(t=>t.denom===e);return t?t.amount:new n.Int(0)}static exclude(t,e){return t.filter(t=>0===e.indexOf(t.denom))}static concat(...t){return 0===t.length?[]:t.slice().reduce((t,e)=>{var i=t.find(t=>t.denom===e.denom);return i?(i=new s.Coin(i.denom,i.amount.add(e.amount)),t.push(i)):(i=new s.Coin(e.denom,e.amount),t.push(i)),t},[])}static getCoinFromDecimals(t,e,i){var n=t.find(t=>t.coinDenom===i);if(!n)throw new Error("Invalid currency");let r=new c.Dec(1);for(let t=0;t<n.coinDecimals;t++)r=r.mul(new c.Dec(10));let o=new c.Dec(e);if(o=o.mul(r),new c.Dec(o.truncate()).equals(o))return new s.Coin(n.coinMinimalDenom,o.truncate());throw new Error("Can't divide anymore")}static parseDecAndDenomFromCoin(t,e){let i=t.find(t=>t.coinMinimalDenom===e.denom),n=(i=i||{coinDecimals:0,coinDenom:e.denom,coinMinimalDenom:e.denom},new c.Dec(1));for(let t=0;t<i.coinDecimals;t++)n=n.mul(new c.Dec(10));return{amount:new c.Dec(e.amount).quoTruncate(n).toString(i.coinDecimals),denom:i.coinDenom}}static shrinkDecimals(t,e,i,n=!1){var r,o;return t.equals(new c.Dec(0))?"0":(r=t.isNegative(),o=t.abs().truncate(),t=t.abs().sub(new c.Dec(o)),e=0===(i=Math.max(i-o.toString().length+1,e))?"":t.toString(i).replace("0.",""),(r?"-":"")+(n?a.integerStringToUSLocaleString(o.toString()):o.toString())+(0<e.length?".":"")+e)}static integerStringToUSLocaleString(t){if(0<=t.indexOf("."))throw new Error(t+" is not integer");if("undefined"!=typeof BigInt)return BigInt(t).toLocaleString("en-US");var e=t,i=[];for(let t=e.length;0<t;t-=3)i.push(e.slice(Math.max(0,t-3),t));return i.reverse().join(",")}static coinToTrimmedString(t,e,i=" "){t=new c.Dec(t.amount).quoTruncate(r.DecUtils.getPrecisionDec(e.coinDecimals));return""+r.DecUtils.trim(t)+i+e.coinDenom}}e.CoinUtils=a},394:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.Coin=void 0;const n=i(140);e.Coin=class r{constructor(t,e){this.denom=t,this.amount=e instanceof n.Int?e:new n.Int(e)}static parse(t){var e;if((t=new RegExp("([0-9]+)[ ]*([a-zA-Z]+)$").exec(t))&&3===t.length)return e=t[2],t=t[1],new r(e,t);throw new Error("Invalid coin str")}toString(){return""+this.amount.toString()+this.denom}}},395:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.exponentDecStringToDecString=e.isExponentDecString=e.isValidDecimalString=e.isValidIntegerString=void 0;const n=/^-?\d+$/,r=/^-?\d+.?\d*$/,s=/^(-?)([\d.]+)e([-+])([\d]+)$/;function c(e){let i="";for(let t=0;t<e;t++)i+="0";return i}e.isValidIntegerString=function(t){return n.test(t)},e.isValidDecimalString=function(t){return r.test(t)},e.isExponentDecString=function(t){return s.test(t)},e.exponentDecStringToDecString=function(t){var e=t.split(s);if(6!==e.length)return t;t="-"===e[1];let i=e[2];var n=i.indexOf("."),r=e[4];let o=parseInt(r)*("-"===e[3]?-1:1);return 0<=n&&(r=i.length-n-1,o-=r,i=function(t){for(;0<t.length&&"0"===t[0];)t=t.slice(1);return 0===t.length||"."===t[0]?"0"+t:t}(i.replace(".",""))),e=t?"-":"",o<0?i.length>-o?(n=i.length+o,e+i.slice(0,n)+"."+i.slice(n)):e+"0."+c(-(i.length+o))+i:e+i+c(o)}},658:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.CoinPretty=void 0;const n=i(210),r=i(141),o=i(175);e.CoinPretty=class s{constructor(t,e){this._currency=t,this.amount=e,this._options={separator:" ",upperCase:!1,lowerCase:!1,hideDenom:!1,hideIBCMetadata:!1},"object"==typeof this.amount&&"toDec"in this.amount?this.amount=this.amount.toDec():this.amount instanceof r.Dec||(this.amount=new r.Dec(this.amount)),this.intPretty=new n.IntPretty(this.amount.quoTruncate(o.DecUtils.getTenExponentNInPrecisionRange(t.coinDecimals))).maxDecimals(t.coinDecimals)}get options(){return Object.assign(Object.assign({},this._options),this.intPretty.options)}get denom(){return this.currency.coinDenom}get currency(){return this._currency}setCurrency(t){var e=this.clone();return e.intPretty=this.intPretty.moveDecimalPointRight(this._currency.coinDecimals-t.coinDecimals),e._currency=t,e}separator(t){var e=this.clone();return e._options.separator=t,e}upperCase(t){var e=this.clone();return e._options.upperCase=t,e._options.lowerCase=!t,e}lowerCase(t){var e=this.clone();return e._options.lowerCase=t,e._options.upperCase=!t,e}hideDenom(t){var e=this.clone();return e._options.hideDenom=t,e}hideIBCMetadata(t){var e=this.clone();return e._options.hideIBCMetadata=t,e}moveDecimalPointLeft(t){var e=this.clone();return e.intPretty=e.intPretty.moveDecimalPointLeft(t),e}moveDecimalPointRight(t){var e=this.clone();return e.intPretty=e.intPretty.moveDecimalPointRight(t),e}increasePrecision(t){return this.moveDecimalPointLeft(t)}decreasePrecision(t){return this.moveDecimalPointRight(t)}maxDecimals(t){var e=this.clone();return e.intPretty=e.intPretty.maxDecimals(t),e}inequalitySymbol(t){var e=this.clone();return e.intPretty=e.intPretty.inequalitySymbol(t),e}inequalitySymbolSeparator(t){var e=this.clone();return e.intPretty=e.intPretty.inequalitySymbolSeparator(t),e}trim(t){var e=this.clone();return e.intPretty=e.intPretty.trim(t),e}shrink(t){var e=this.clone();return e.intPretty=e.intPretty.shrink(t),e}locale(t){var e=this.clone();return e.intPretty=e.intPretty.locale(t),e}ready(t){var e=this.clone();return e.intPretty=e.intPretty.ready(t),e}get isReady(){return this.intPretty.isReady}add(t){var e=t instanceof s;if(e&&"currency"in t&&this.currency.coinMinimalDenom!==t.currency.coinMinimalDenom)return this.clone();"toDec"in t&&(t=t.toDec());var i=this.clone();return i.intPretty=i.intPretty.add(e?t:t.mul(o.DecUtils.getTenExponentNInPrecisionRange(-this._currency.coinDecimals))),i}sub(t){var e=t instanceof s;if(e&&"currency"in t&&this.currency.coinMinimalDenom!==t.currency.coinMinimalDenom)return this.clone();"toDec"in t&&(t=t.toDec());var i=this.clone();return i.intPretty=i.intPretty.sub(e?t:t.mul(o.DecUtils.getTenExponentNInPrecisionRange(-this._currency.coinDecimals))),i}mul(t){var e=this.clone();return e.intPretty=e.intPretty.mul(t),e}quo(t){var e=this.clone();return e.intPretty=e.intPretty.quo(t),e}toDec(){return this.intPretty.toDec()}toCoin(){var t=this.toDec().mulTruncate(o.DecUtils.getTenExponentNInPrecisionRange(this.currency.coinDecimals)).truncate();return{denom:this.currency.coinMinimalDenom,amount:t.toString()}}toString(){let t=this.denom,e=(this._options.hideIBCMetadata&&"originCurrency"in this.currency&&this.currency.originCurrency&&(t=this.currency.originCurrency.coinDenom),this._options.upperCase&&(t=t.toUpperCase()),this._options.lowerCase&&(t=t.toLowerCase()),this._options.separator);return this._options.hideDenom&&(t="",e=""),this.intPretty.toStringWithSymbols("",""+e+t)}clone(){var t=new s(this._currency,this.amount);return t._options=Object.assign({},this._options),t.intPretty=this.intPretty.clone(),t}}},659:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.PricePretty=void 0;const n=i(210),r=i(175);e.PricePretty=class o{constructor(t,e){this._fiatCurrency=t,this.amount=e,this._options={separator:"",upperCase:!1,lowerCase:!1,locale:"en-US"},this.intPretty=new n.IntPretty(e).maxDecimals(t.maxDecimals).shrink(!0).trim(!0).locale(!1).inequalitySymbol(!0),this._options.locale=t.locale}get options(){return Object.assign(Object.assign({},this.intPretty.options),this._options)}get symbol(){return this._fiatCurrency.symbol}get fiatCurrency(){return this._fiatCurrency}separator(t){var e=this.clone();return e._options.separator=t,e}upperCase(t){var e=this.clone();return e._options.upperCase=t,e._options.lowerCase=!t,e}lowerCase(t){var e=this.clone();return e._options.lowerCase=t,e._options.upperCase=!t,e}moveDecimalPointLeft(t){var e=this.clone();return e.intPretty=e.intPretty.moveDecimalPointLeft(t),e}moveDecimalPointRight(t){var e=this.clone();return e.intPretty=e.intPretty.moveDecimalPointRight(t),e}increasePrecision(t){return this.moveDecimalPointLeft(t)}decreasePrecision(t){return this.moveDecimalPointRight(t)}maxDecimals(t){var e=this.clone();return e.intPretty=e.intPretty.maxDecimals(t),e}inequalitySymbol(t){var e=this.clone();return e.intPretty=e.intPretty.inequalitySymbol(t),e}inequalitySymbolSeparator(t){var e=this.clone();return e.intPretty=e.intPretty.inequalitySymbolSeparator(t),e}trim(t){var e=this.clone();return e.intPretty=e.intPretty.trim(t),e}shrink(t){var e=this.clone();return e.intPretty=e.intPretty.shrink(t),e}locale(t){var e=this.clone();return e._options.locale=t,e}ready(t){var e=this.clone();return e.intPretty=e.intPretty.ready(t),e}get isReady(){return this.intPretty.isReady}add(t){var e=this.clone();return e.intPretty=e.intPretty.add(t),e}sub(t){var e=this.clone();return e.intPretty=e.intPretty.sub(t),e}mul(t){var e=this.clone();return e.intPretty=e.intPretty.mul(t),e}quo(t){var e=this.clone();return e.intPretty=e.intPretty.quo(t),e}toDec(){return this.intPretty.toDec()}toString(){let t=this.symbol;this._options.upperCase&&(t=t.toUpperCase()),this._options.lowerCase&&(t=t.toLowerCase());var e=this.toDec(),i=this.options;if(i.inequalitySymbol&&!e.isZero()&&e.abs().lt(r.DecUtils.getTenExponentN(-i.maxDecimals)))return this.intPretty.toStringWithSymbols(""+t+this._options.separator,"");let n=parseFloat(this.intPretty.toString()).toLocaleString(i.locale,{maximumFractionDigits:i.maxDecimals});return(e="-"===n.charAt(0))&&(n=n.slice(1)),(e?"-":"")+t+this._options.separator+n}clone(){var t=new o(this._fiatCurrency,this.amount);return t._options=Object.assign({},this._options),t.intPretty=this.intPretty.clone(),t}}},660:function(t,e,i){"use strict";Object.defineProperty(e,"__esModule",{value:!0}),e.RatePretty=void 0;const n=i(210);e.RatePretty=class r{constructor(t){this.amount=t,this._options={separator:"",symbol:"%"},this.intPretty=new n.IntPretty(t),this.intPretty=this.intPretty.maxDecimals(3).shrink(!1).trim(!0).locale(!0).inequalitySymbol(!0)}get options(){return Object.assign(Object.assign({},this.intPretty.options),this._options)}separator(t){var e=this.clone();return e._options.separator=t,e}symbol(t){var e=this.clone();return e._options.symbol=t,e}moveDecimalPointLeft(t){var e=this.clone();return e.intPretty=e.intPretty.moveDecimalPointLeft(t),e}moveDecimalPointRight(t){var e=this.clone();return e.intPretty=e.intPretty.moveDecimalPointRight(t),e}maxDecimals(t){var e=this.clone();return e.intPretty=e.intPretty.maxDecimals(t),e}inequalitySymbol(t){var e=this.clone();return e.intPretty=e.intPretty.inequalitySymbol(t),e}inequalitySymbolSeparator(t){var e=this.clone();return e.intPretty=e.intPretty.inequalitySymbolSeparator(t),e}trim(t){var e=this.clone();return e.intPretty=e.intPretty.trim(t),e}shrink(t){var e=this.clone();return e.intPretty=e.intPretty.shrink(t),e}locale(t){var e=this.clone();return e.intPretty=e.intPretty.locale(t),e}ready(t){var e=this.clone();return e.intPretty=e.intPretty.ready(t),e}get isReady(){return this.intPretty.isReady}add(t){var e=this.clone();return e.intPretty=e.intPretty.add(t),e}sub(t){var e=this.clone();return e.intPretty=e.intPretty.sub(t),e}mul(t){var e=this.clone();return e.intPretty=e.intPretty.mul(t),e}quo(t){var e=this.clone();return e.intPretty=e.intPretty.quo(t),e}toDec(){return this.intPretty.toDec()}toString(){return this.intPretty.moveDecimalPointRight(2).toStringWithSymbols("",""+this._options.separator+this._options.symbol)}clone(){var t=new r(this.amount);return t._options=Object.assign({},this._options),t.intPretty=this.intPretty.clone(),t}}},662:function(t,e){},705:function(t,e){},707:function(t,e){},717:function(t,e){},719:function(t,e){},745:function(t,e){},790:function(t,e){},792:function(t,e){},799:function(t,e){},800:function(t,e){},865:function(t,e){},887:function(t,e){},96:function(t,e,i){"use strict";var n=this&&this.__createBinding||(Object.create?function(t,e,i,n){void 0===n&&(n=i),Object.defineProperty(t,n,{enumerable:!0,get:function(){return e[i]}})}:function(t,e,i,n){t[n=void 0===n?i:n]=e[i]}),r=this&&this.__exportStar||function(t,e){for(var i in t)"default"===i||Object.prototype.hasOwnProperty.call(e,i)||n(e,t,i)};Object.defineProperty(e,"__esModule",{value:!0}),r(i(1029),e),r(i(1030),e),r(i(1031),e),r(i(1032),e),r(i(1033),e),r(i(1035),e),r(i(1036),e),r(i(1037),e)}}]);