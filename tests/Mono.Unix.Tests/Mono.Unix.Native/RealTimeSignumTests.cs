//
// RealTimeSignumTests.cs - NUnit Test Cases for Mono.Unix.Native.RealTimeSignum
//
// Authors:
//	Tim Jenks  <tim.jenks@realtimeworlds.com>
//
// (C) 2008 Realtime Worlds Ltd
//

using Microsoft.VisualStudio.TestTools.UnitTesting;

using System;
using System.Text;
using System.Threading;
using Mono.Unix;
using Mono.Unix.Android;
using Mono.Unix.Native;

namespace MonoTests.Mono.Unix.Native {

	[TestClass]
	[TestCategory ("NotOnMac"), TestCategory ("NotOnWindows")]
	public class RealTimeSignumTest 
	{
		[TestMethod]
		public void TestRealTimeOutOfRange ()
		{
			if (!TestHelper.CanUseRealTimeSignals ())
				return;

			Assert.Throws<ArgumentOutOfRangeException> (() => {
				RealTimeSignum rts = new RealTimeSignum (int.MaxValue);
			});
		}

		[TestMethod]
		public void TestRealTimeSignumNegativeOffset ()
		{
			if (!TestHelper.CanUseRealTimeSignals ())
				return;

			Assert.Throws<ArgumentOutOfRangeException> (() => {
				RealTimeSignum rts1 = new RealTimeSignum (-1);
			});
		}

		[TestMethod]
		public void TestRTSignalEquality ()
		{
			if (!TestHelper.CanUseRealTimeSignals ())
				return;
			RealTimeSignum rts1 = new RealTimeSignum (0);
			RealTimeSignum rts2 = new RealTimeSignum (0);
			Assert.IsTrue (rts1 == rts2);
			Assert.IsFalse (rts1 != rts2);
		}

		[TestMethod]
		public void TestRTSignalInequality ()
		{
			if (!TestHelper.CanUseRealTimeSignals ())
				return;
			RealTimeSignum rts1 = new RealTimeSignum (0);
			RealTimeSignum rts2 = new RealTimeSignum (1);
			Assert.IsFalse (rts1 == rts2);
			Assert.IsTrue (rts1 != rts2);
		}

		[TestMethod]
		public void TestRTSignalGetHashCodeEquality ()
		{
			if (!TestHelper.CanUseRealTimeSignals ())
				return;
			RealTimeSignum rts1 = new RealTimeSignum (0);
			RealTimeSignum rts2 = new RealTimeSignum (0);
			Assert.AreEqual (rts2.GetHashCode (), rts1.GetHashCode ()));
		}

		[TestMethod]
		public void TestRTSignalGetHashCodeInequality ()
		{
			if (!TestHelper.CanUseRealTimeSignals ())
				return;
			RealTimeSignum rts1 = new RealTimeSignum (0);
			RealTimeSignum rts2 = new RealTimeSignum (1);
			Assert.AreNotEqual (rts2.GetHashCode (), rts1.GetHashCode ());
		}

		[TestMethod]
		public void TestIsRTSignalPropertyForRTSignum ()
		{
			if (!TestHelper.CanUseRealTimeSignals ())
				return;

			int rtsigOffset = Utilities.FindFirstFreeRTSignal ();
			Assert.IsGreaterThan (-1, rtsigOffset, "No available RT signals");

			UnixSignal signal1 = new UnixSignal(new RealTimeSignum (rtsigOffset));
			Assert.IsTrue (signal1.IsRealTimeSignal);
		}

		[TestMethod]
		public void TestIsRTSignalPropertyForSignum ()
		{
			if (!TestHelper.CanUseRealTimeSignals ())
				return;
			UnixSignal signal1 = new UnixSignal (Signum.SIGSEGV);
			Assert.IsFalse (signal1.IsRealTimeSignal);
		}

	}
}
