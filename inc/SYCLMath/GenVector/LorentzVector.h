// @(#)root/mathcore:$Id$
// Authors: W. Brown, M. Fischler, L. Moneta    2005

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2005 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class LorentzVector
//
// Created by:    moneta   at Tue May 31 17:06:09 2005
// Major mods by: fischler at Wed Jul 20   2005
//
// Last update: $Id$
//
#ifndef ROOT_Experimental_GenVector_LorentzVector
#define ROOT_Experimental_GenVector_LorentzVector  1

#include "SYCLMath/GenVector/MathUtil.h"

#include "SYCLMath/GenVector/PxPyPzE4D.h"

#include "SYCLMath/GenVector/DisplacementVector3D.h"

#include "SYCLMath/GenVector/GenVectorIO.h"

#include <cmath>
#include <string>

namespace ROOT {

  namespace Experimental {

//__________________________________________________________________________________________
/** @ingroup GenVector

Class describing a generic LorentzVector in the 4D space-time,
using the specified coordinate system for the spatial vector part.
The metric used for the LorentzVector is (-,-,-,+).
In the case of LorentzVector we don't distinguish the concepts
of points and displacement vectors as in the 3D case,
since the main use case for 4D Vectors is to describe the kinematics of
relativistic particles. A LorentzVector behaves like a
DisplacementVector in 4D.  The Minkowski components could be viewed as
v and t, or for kinematic 4-vectors, as p and E.

ROOT provides specialisations and aliases to them of the ROOT::Experimental::LorentzVector template:
- ROOT::Experimental::PtEtaPhiMVector based on pt (rho),eta,phi and M (t) coordinates in double precision
- ROOT::Experimental::PtEtaPhiEVector based on pt (rho),eta,phi and E (t) coordinates in double precision
- ROOT::Experimental::PxPyPzMVector based on px,py,pz and M (mass) coordinates in double precision
- ROOT::Experimental::PxPyPzEVector based on px,py,pz and E (energy) coordinates in double precision
- ROOT::Experimental::XYZTVector based on x,y,z,t coordinates (cartesian) in double precision (same as PxPyPzEVector)
- ROOT::Experimental::XYZTVectorF based on x,y,z,t coordinates (cartesian) in float precision (same as PxPyPzEVector but float)

@sa Overview of the @ref GenVector "physics vector library"
*/

    template< class CoordSystem >
    class LorentzVector {

    public:

       // ------ ctors ------

       typedef typename CoordSystem::Scalar Scalar;
       typedef CoordSystem CoordinateType;

       /**
          default constructor of an empty vector (Px = Py = Pz = E = 0 )
       */
        __roohost__ __roodevice__ LorentzVector ( ) : fCoordinates() { }

       /**
          generic constructors from four scalar values.
          The association between values and coordinate depends on the
          coordinate system.  For PxPyPzE4D,
          \param a scalar value (Px)
          \param b scalar value (Py)
          \param c scalar value (Pz)
          \param d scalar value (E)
       */
        __roohost__ __roodevice__ LorentzVector(const Scalar & a,
                     const Scalar & b,
                     const Scalar & c,
                     const Scalar & d) :
          fCoordinates(a , b,  c, d)  { }

       /**
          constructor from a LorentzVector expressed in different
          coordinates, or using a different Scalar type
       */
       template< class Coords >
         __roohost__ __roodevice__ explicit LorentzVector(const LorentzVector<Coords> & v ) :
          fCoordinates( v.Coordinates() ) { }

       /**
          Construct from a foreign 4D vector type, for example, HepLorentzVector
          Precondition: v must implement methods x(), y(), z(), and t()
       */
       template<class ForeignLorentzVector,
                typename = decltype(std::declval<ForeignLorentzVector>().x()
                                    + std::declval<ForeignLorentzVector>().y()
                                    + std::declval<ForeignLorentzVector>().z()
                                    + std::declval<ForeignLorentzVector>().t())>
         __roohost__ __roodevice__ explicit LorentzVector( const ForeignLorentzVector & v) :
          fCoordinates(PxPyPzE4D<Scalar>( v.x(), v.y(), v.z(), v.t()  ) ) { }

#ifdef LATER
       /**
          construct from a generic linear algebra  vector implementing operator []
          and with a size of at least 4. This could be also a C array
          In this case v[0] is the first data member
          ( Px for a PxPyPzE4D base)
          \param v LA vector
          \param index0 index of first vector element (Px)
       */
       template< class LAVector >
         __roohost__ __roodevice__ explicit LorentzVector(const LAVector & v, size_t index0 ) {
          fCoordinates = CoordSystem ( v[index0], v[index0+1], v[index0+2], v[index0+3] );
       }
#endif


       // ------ assignment ------

       /**
          Assignment operator from a lorentz vector of arbitrary type
       */
       template< class OtherCoords >
        __roohost__ __roodevice__ LorentzVector & operator= ( const LorentzVector<OtherCoords> & v) {
          fCoordinates = v.Coordinates();
          return *this;
       }

       /**
          assignment from any other Lorentz vector  implementing
          x(), y(), z() and t()
       */
       template<class ForeignLorentzVector,
                typename = decltype(std::declval<ForeignLorentzVector>().x()
                                    + std::declval<ForeignLorentzVector>().y()
                                    + std::declval<ForeignLorentzVector>().z()
                                    + std::declval<ForeignLorentzVector>().t())>
        __roohost__ __roodevice__ LorentzVector & operator = ( const ForeignLorentzVector & v) {
          SetXYZT( v.x(), v.y(), v.z(), v.t() );
          return *this;
       }

#ifdef LATER
       /**
          assign from a generic linear algebra  vector implementing operator []
          and with a size of at least 4
          In this case v[0] is the first data member
          ( Px for a PxPyPzE4D base)
          \param v LA vector
          \param index0 index of first vector element (Px)
       */
       template< class LAVector >
       __roohost__ __roodevice__ LorentzVector & AssignFrom(const LAVector & v, size_t index0=0 ) {
          fCoordinates.SetCoordinates( v[index0], v[index0+1], v[index0+2], v[index0+3] );
          return *this;
       }
#endif

       // ------ Set, Get, and access coordinate data ------

       /**
          Retrieve a const reference to  the coordinates object
       */
        __roohost__ __roodevice__ const CoordSystem &  Coordinates() const {
          return fCoordinates;
       }

       /**
          Set internal data based on an array of 4 Scalar numbers
       */
       __roohost__ __roodevice__ LorentzVector<CoordSystem>& SetCoordinates( const Scalar src[] ) {
          fCoordinates.SetCoordinates(src);
          return *this;
       }

       /**
          Set internal data based on 4 Scalar numbers
       */
       __roohost__ __roodevice__ LorentzVector<CoordSystem>& SetCoordinates( Scalar a, Scalar b, Scalar c, Scalar d ) {
          fCoordinates.SetCoordinates(a, b, c, d);
          return *this;
       }

       /**
          Set internal data based on 4 Scalars at *begin to *end
       */
       template< class IT >
       __roohost__ __roodevice__ LorentzVector<CoordSystem>& SetCoordinates( IT begin, IT end  ) {
          IT a = begin; IT b = ++begin; IT c = ++begin; IT d = ++begin;
          (void)end;
          assert (++begin==end);
          SetCoordinates (*a,*b,*c,*d);
          return *this;
       }

       /**
          get internal data into 4 Scalar numbers
       */
       __roohost__ __roodevice__ void GetCoordinates( Scalar& a, Scalar& b, Scalar& c, Scalar & d ) const
       { fCoordinates.GetCoordinates(a, b, c, d);  }

       /**
          get internal data into an array of 4 Scalar numbers
       */
       __roohost__ __roodevice__ void GetCoordinates( Scalar dest[] ) const
       { fCoordinates.GetCoordinates(dest);  }

       /**
          get internal data into 4 Scalars at *begin to *end
       */
       template <class IT>
       __roohost__ __roodevice__ void GetCoordinates( IT begin, IT end ) const
       { IT a = begin; IT b = ++begin; IT c = ++begin; IT d = ++begin;
       (void)end;
       assert (++begin==end);
       GetCoordinates (*a,*b,*c,*d);
       }

       /**
          get internal data into 4 Scalars at *begin
       */
       template <class IT>
       __roohost__ __roodevice__ void GetCoordinates( IT begin ) const {
          Scalar a,b,c,d = 0;
          GetCoordinates (a,b,c,d);
          *begin++ = a;
          *begin++ = b;
          *begin++ = c;
          *begin   = d;
       }

       /**
          set the values of the vector from the cartesian components (x,y,z,t)
          (if the vector is held in another coordinates, like (Pt,eta,phi,m)
          then (x, y, z, t) are converted to that form)
       */
       __roohost__ __roodevice__ LorentzVector<CoordSystem>& SetXYZT (Scalar xx, Scalar yy, Scalar zz, Scalar tt) {
          fCoordinates.SetPxPyPzE(xx,yy,zz,tt);
          return *this;
       }
       __roohost__ __roodevice__ LorentzVector<CoordSystem>& SetPxPyPzE (Scalar xx, Scalar yy, Scalar zz, Scalar ee) {
          fCoordinates.SetPxPyPzE(xx,yy,zz,ee);
          return *this;
       }

       // ------------------- Equality -----------------

       /**
          Exact equality
       */
       __roohost__ __roodevice__ bool operator==(const LorentzVector & rhs) const {
          return fCoordinates==rhs.fCoordinates;
       }
       __roohost__ __roodevice__ bool operator!= (const LorentzVector & rhs) const {
          return !(operator==(rhs));
       }

       // ------ Individual element access, in various coordinate systems ------

       // individual coordinate accessors in various coordinate systems

       /**
          spatial X component
       */
       __roohost__ __roodevice__ Scalar Px() const  { return fCoordinates.Px(); }
       __roohost__ __roodevice__ Scalar X()  const  { return fCoordinates.Px(); }
       /**
          spatial Y component
       */
       __roohost__ __roodevice__ Scalar Py() const { return fCoordinates.Py(); }
       __roohost__ __roodevice__ Scalar Y()  const { return fCoordinates.Py(); }
       /**
          spatial Z component
       */
       __roohost__ __roodevice__ Scalar  Pz() const { return fCoordinates.Pz(); }
       __roohost__ __roodevice__ Scalar  Z()  const { return fCoordinates.Pz(); }
       /**
          return 4-th component (time, or energy for a 4-momentum vector)
       */
       __roohost__ __roodevice__ Scalar  E()  const { return fCoordinates.E(); }
       __roohost__ __roodevice__ Scalar  T()  const { return fCoordinates.E(); }
       /**
          return magnitude (mass) squared  M2 = T**2 - X**2 - Y**2 - Z**2
          (we use -,-,-,+ metric)
       */
       __roohost__ __roodevice__ Scalar  M2()   const { return fCoordinates.M2(); }
       /**
          return magnitude (mass) using the  (-,-,-,+)  metric.
          If M2 is negative (space-like vector) a GenVector_exception
          is suggested and if continuing, - sqrt( -M2) is returned
       */
       __roohost__ __roodevice__ Scalar  M() const    { return fCoordinates.M();}
       /**
          return the spatial (3D) magnitude ( sqrt(X**2 + Y**2 + Z**2) )
       */
       __roohost__ __roodevice__ Scalar  R() const { return fCoordinates.R(); }
       __roohost__ __roodevice__ Scalar  P() const { return fCoordinates.R(); }
       /**
          return the square of the spatial (3D) magnitude ( X**2 + Y**2 + Z**2 )
       */
       __roohost__ __roodevice__ Scalar  P2() const { return P() * P(); }
       /**
          return the square of the transverse spatial component ( X**2 + Y**2 )
       */
       __roohost__ __roodevice__ Scalar  Perp2( ) const { return fCoordinates.Perp2();}

       /**
          return the  transverse spatial component sqrt ( X**2 + Y**2 )
       */
       __roohost__ __roodevice__ Scalar  Pt()  const { return fCoordinates.Pt(); }
       __roohost__ __roodevice__ Scalar  Rho() const { return fCoordinates.Pt(); }

       /**
          return the transverse mass squared
          \f[ m_t^2 = E^2 - p{_z}^2 \f]
       */
       __roohost__ __roodevice__ Scalar  Mt2() const { return fCoordinates.Mt2(); }

       /**
          return the transverse mass
          \f[ \sqrt{ m_t^2 = E^2 - p{_z}^2} X sign(E^ - p{_z}^2) \f]
       */
       __roohost__ __roodevice__ Scalar  Mt() const { return fCoordinates.Mt(); }

       /**
          return the transverse energy squared
          \f[ e_t = \frac{E^2 p_{\perp}^2 }{ |p|^2 } \f]
       */
       __roohost__ __roodevice__ Scalar  Et2() const { return fCoordinates.Et2(); }

       /**
          return the transverse energy
          \f[ e_t = \sqrt{ \frac{E^2 p_{\perp}^2 }{ |p|^2 } } X sign(E) \f]
       */
       __roohost__ __roodevice__ Scalar  Et() const { return fCoordinates.Et(); }

       /**
          azimuthal  Angle
       */
       __roohost__ __roodevice__ Scalar  Phi() const  { return fCoordinates.Phi();}

       /**
          polar Angle
       */
       __roohost__ __roodevice__ Scalar  Theta() const { return fCoordinates.Theta(); }

       /**
          pseudorapidity
          \f[ \eta = - \ln { \tan { \frac { \theta} {2} } } \f]
       */
       __roohost__ __roodevice__ Scalar  Eta() const { return fCoordinates.Eta(); }

       /**
          get the spatial components of the Vector in a
          DisplacementVector based on Cartesian Coordinates
       */
       ::ROOT::Experimental::DisplacementVector3D<Cartesian3D<Scalar> > Vect() const {
          return ::ROOT::Experimental::DisplacementVector3D<Cartesian3D<Scalar> >( X(), Y(), Z() );
       }

       // ------ Operations combining two Lorentz vectors ------

       /**
          scalar (Dot) product of two LorentzVector vectors (metric is -,-,-,+)
          Enable the product using any other LorentzVector implementing
          the x(), y() , y() and t() member functions
          \param  q  any LorentzVector implementing the x(), y() , z() and t()
          member functions
          \return the result of v.q of type according to the base scalar type of v
       */

       template< class OtherLorentzVector >
       __roohost__ __roodevice__ Scalar  Dot(const OtherLorentzVector & q) const {
          return t()*q.t() - x()*q.x() - y()*q.y() - z()*q.z();
       }

       /**
          Self addition with another Vector ( v+= q )
          Enable the addition with any other LorentzVector
          \param  q  any LorentzVector implementing the x(), y() , z() and t()
          member functions
       */
      template< class OtherLorentzVector >
      inline __roohost__ __roodevice__ LorentzVector & operator += ( const OtherLorentzVector & q)
       {
          SetXYZT( x() + q.x(), y() + q.y(), z() + q.z(), t() + q.t()  );
          return *this;
       }

       /**
          Self subtraction of another Vector from this ( v-= q )
          Enable the addition with any other LorentzVector
          \param  q  any LorentzVector implementing the x(), y() , z() and t()
          member functions
       */
       template< class OtherLorentzVector >
       LorentzVector & operator -= ( const OtherLorentzVector & q) {
          SetXYZT( x() - q.x(), y() - q.y(), z() - q.z(), t() - q.t()  );
          return *this;
       }

       /**
          addition of two LorentzVectors (v3 = v1 + v2)
          Enable the addition with any other LorentzVector
          \param  v2  any LorentzVector implementing the x(), y() , z() and t()
          member functions
          \return a new LorentzVector of the same type as v1
       */
       template<class OtherLorentzVector>
       __roohost__ __roodevice__ LorentzVector  operator +  ( const OtherLorentzVector & v2) const
       {
          LorentzVector<CoordinateType> v3(*this);
          v3 += v2;
          return v3;
       }

       /**
          subtraction of two LorentzVectors (v3 = v1 - v2)
          Enable the subtraction of any other LorentzVector
          \param  v2  any LorentzVector implementing the x(), y() , z() and t()
          member functions
          \return a new LorentzVector of the same type as v1
       */
       template<class OtherLorentzVector>
       __roohost__ __roodevice__ LorentzVector   operator -  ( const OtherLorentzVector & v2) const {
          LorentzVector<CoordinateType> v3(*this);
          v3 -= v2;
          return v3;
       }

       //--- scaling operations ------

       /**
          multiplication by a scalar quantity v *= a
       */
       __roohost__ __roodevice__ LorentzVector  & operator *= (Scalar a) {
          fCoordinates.Scale(a);
          return *this;
       }

       /**
          division by a scalar quantity v /= a
       */
       __roohost__ __roodevice__ LorentzVector  & operator /= (Scalar a) {
          fCoordinates.Scale(1/a);
          return *this;
       }

       /**
          product of a LorentzVector by a scalar quantity
          \param a  scalar quantity of type a
          \return a new mathcoreLorentzVector q = v * a same type as v
       */
       __roohost__ __roodevice__ LorentzVector  operator * ( const Scalar & a) const {
          LorentzVector tmp(*this);
          tmp *= a;
          return tmp;
       }

       /**
          Divide a LorentzVector by a scalar quantity
          \param a  scalar quantity of type a
          \return a new mathcoreLorentzVector q = v / a same type as v
       */
       __roohost__ __roodevice__ LorentzVector <CoordSystem> operator / ( const Scalar & a) const {
          LorentzVector<CoordSystem> tmp(*this);
          tmp /= a;
          return tmp;
       }

       /**
          Negative of a LorentzVector (q = - v )
          \return a new LorentzVector with opposite direction and time
       */
       __roohost__ __roodevice__ LorentzVector  operator - () const {
          //LorentzVector<CoordinateType> v(*this);
          //v.Negate();
          return operator*( Scalar(-1) );
       }
       __roohost__ __roodevice__ LorentzVector  operator + () const {
          return *this;
       }

       // ---- Relativistic Properties ----

       /**
          Rapidity relative to the Z axis:  .5 log [(E+Pz)/(E-Pz)]
       */
       __roohost__ __roodevice__ Scalar Rapidity() const {
          // TODO - It would be good to check that E > Pz and use the Throw()
          //        mechanism or at least load a NAN if not.
          //        We should then move the code to a .cpp file.
          const Scalar ee  = E();
          const Scalar ppz = Pz();
          return Scalar(0.5) * mylog((ee + ppz) / (ee - ppz));
       }

       /**
          Rapidity in the direction of travel: atanh (|P|/E)=.5 log[(E+P)/(E-P)]
       */
       __roohost__ __roodevice__ Scalar ColinearRapidity() const {
          // TODO - It would be good to check that E > P and use the Throw()
          //        mechanism or at least load a NAN if not.
          const Scalar ee = E();
          const Scalar pp = P();
          return Scalar(0.5) * mylog((ee + pp) / (ee - pp));
       }

       /**
          Determine if momentum-energy can represent a physical massive particle
       */
       __roohost__ __roodevice__ bool isTimelike( ) const {
          Scalar ee = E(); Scalar pp = P(); return ee*ee > pp*pp;
       }

       /**
          Determine if momentum-energy can represent a massless particle
       */
       bool isLightlike( Scalar tolerance
                         = 100*std::numeric_limits<Scalar>::epsilon() ) const {
          Scalar ee = E(); Scalar pp = P(); Scalar delta = ee-pp;
          if ( ee==0 ) return pp==0;
          return delta*delta < tolerance * ee*ee;
       }

       /**
          Determine if momentum-energy is spacelike, and represents a tachyon
       */
       bool isSpacelike( ) const {
          Scalar ee = E(); Scalar pp = P(); return ee*ee < pp*pp;
       }

       typedef DisplacementVector3D< Cartesian3D<Scalar> > BetaVector;

       /**
          The beta vector for the boost that would bring this vector into
          its center of mass frame (zero momentum)
       */
       __roohost__ __roodevice__ BetaVector BoostToCM( ) const {
          if (E() == 0) {
             if (P() == 0) {
                return BetaVector();
             } else {
                // TODO - should attempt to Throw with msg about
                // boostVector computed for LorentzVector with t=0
                return -Vect()/E();
             }
          }
          if (M2() <= 0) {
             // TODO - should attempt to Throw with msg about
             // boostVector computed for a non-timelike LorentzVector
          }
          return -Vect()/E();
       }

       /**
          The beta vector for the boost that would bring this vector into
          its center of mass frame (zero momentum)
       */
       template <class Other4Vector>
       __roohost__ __roodevice__ BetaVector BoostToCM(const Other4Vector& v ) const {
          Scalar eSum = E() + v.E();
          DisplacementVector3D< Cartesian3D<Scalar> > vecSum = Vect() + v.Vect();
          if (eSum == 0) {
             if (vecSum.Mag2() == 0) {
                return BetaVector();
             } else {
                // TODO - should attempt to Throw with msg about
                // boostToCM computed for two 4-vectors with combined t=0
                return BetaVector(vecSum/eSum);
             }
             // TODO - should attempt to Throw with msg about
             // boostToCM computed for two 4-vectors with combined e=0
          }
          return BetaVector (vecSum * (-1./eSum));
       }

       //beta and gamma

       /**
           Return beta scalar value
       */
       __roohost__ __roodevice__ Scalar Beta() const {
          if ( E() == 0 ) {
             if ( P2() == 0)
                // to avoid Nan
                return 0;
             else {
                //GenVector::Throw ("LorentzVector::Beta() - beta computed for LorentzVector with t = 0. Return an Infinite result");
                return 1./E();
             }
          }
          if ( M2() <= 0 ) {
             //GenVector::Throw ("LorentzVector::Beta() - beta computed for non-timelike LorentzVector . Result is physically meaningless" );
          }
          return P() / E();
       }
       /**
           Return Gamma scalar value
       */
       __roohost__ __roodevice__ Scalar Gamma() const {
          const Scalar v2 = P2();
          const Scalar t2 = E() * E();
          if (E() == 0) {
             if ( P2() == 0) {
                return 1;
             } else {
                //GenVector::Throw ("LorentzVector::Gamma() - gamma computed for LorentzVector with t = 0. Return a zero result");

             }
          }
          if ( t2 < v2 ) {
             //GenVector::Throw ("LorentzVector::Gamma() - gamma computed for a spacelike LorentzVector. Imaginary result");
             return 0;
          }
          else if ( t2 == v2 ) {
             //GenVector::Throw ("LorentzVector::Gamma() - gamma computed for a lightlike LorentzVector. Infinite result");
          }
          return Scalar(1) / mysqrt(Scalar(1) - v2 / t2);
       } /* gamma */


       // Method providing limited backward name compatibility with CLHEP ----

       __roohost__ __roodevice__ Scalar  x()     const { return fCoordinates.Px();     }
       __roohost__ __roodevice__ Scalar  y()     const { return fCoordinates.Py();     }
       __roohost__ __roodevice__ Scalar  z()     const { return fCoordinates.Pz();     }
       __roohost__ __roodevice__ Scalar  t()     const { return fCoordinates.E();      }
       __roohost__ __roodevice__ Scalar  px()    const { return fCoordinates.Px();     }
       __roohost__ __roodevice__ Scalar  py()    const { return fCoordinates.Py();     }
       __roohost__ __roodevice__ Scalar  pz()    const { return fCoordinates.Pz();     }
       __roohost__ __roodevice__ Scalar  e()     const { return fCoordinates.E();      }
       __roohost__ __roodevice__ Scalar  r()     const { return fCoordinates.R();      }
       __roohost__ __roodevice__ Scalar  theta() const { return fCoordinates.Theta();  }
       __roohost__ __roodevice__ Scalar  phi()   const { return fCoordinates.Phi();    }
       __roohost__ __roodevice__ Scalar  rho()   const { return fCoordinates.Rho();    }
       __roohost__ __roodevice__ Scalar  eta()   const { return fCoordinates.Eta();    }
       __roohost__ __roodevice__ Scalar  pt()    const { return fCoordinates.Pt();     }
       __roohost__ __roodevice__ Scalar  perp2() const { return fCoordinates.Perp2();  }
       __roohost__ __roodevice__ Scalar  mag2()  const { return fCoordinates.M2();     }
       __roohost__ __roodevice__ Scalar  mag()   const { return fCoordinates.M();      }
       __roohost__ __roodevice__ Scalar  mt()    const { return fCoordinates.Mt();     }
       __roohost__ __roodevice__ Scalar  mt2()   const { return fCoordinates.Mt2();    }


       // Methods  requested by CMS ---
       __roohost__ __roodevice__ Scalar energy() const { return fCoordinates.E();      }
       __roohost__ __roodevice__ Scalar mass()   const { return fCoordinates.M();      }
       __roohost__ __roodevice__ Scalar mass2()  const { return fCoordinates.M2();     }


       /**
          Methods setting a Single-component
          Work only if the component is one of which the vector is represented.
          For example SetE will work for a PxPyPzE Vector but not for a PxPyPzM Vector.
       */
       __roohost__ __roodevice__ LorentzVector <CoordSystem>& SetE  ( Scalar a )  { fCoordinates.SetE  (a); return *this; }
       __roohost__ __roodevice__ LorentzVector <CoordSystem>& SetEta( Scalar a )  { fCoordinates.SetEta(a); return *this; }
       __roohost__ __roodevice__ LorentzVector <CoordSystem>& SetM  ( Scalar a )  { fCoordinates.SetM  (a); return *this; }
       __roohost__ __roodevice__ LorentzVector <CoordSystem>& SetPhi( Scalar a )  { fCoordinates.SetPhi(a); return *this; }
       __roohost__ __roodevice__ LorentzVector <CoordSystem>& SetPt ( Scalar a )  { fCoordinates.SetPt (a); return *this; }
       __roohost__ __roodevice__ LorentzVector <CoordSystem>& SetPx ( Scalar a )  { fCoordinates.SetPx (a); return *this; }
       __roohost__ __roodevice__ LorentzVector <CoordSystem>& SetPy ( Scalar a )  { fCoordinates.SetPy (a); return *this; }
       __roohost__ __roodevice__ LorentzVector <CoordSystem>& SetPz ( Scalar a )  { fCoordinates.SetPz (a); return *this; }

    private:

       CoordSystem  fCoordinates;    // internal coordinate system


    };  // LorentzVector<>



  // global nethods

  /**
     Scale of a LorentzVector with a scalar quantity a
     \param a  scalar quantity of typpe a
     \param v  mathcore::LorentzVector based on any coordinate system
     \return a new mathcoreLorentzVector q = v * a same type as v
   */
    template< class CoordSystem >
     __roohost__ __roodevice__ LorentzVector <CoordSystem> operator *
    ( const typename  LorentzVector<CoordSystem>::Scalar & a,
      const LorentzVector<CoordSystem>& v) {
       LorentzVector<CoordSystem> tmp(v);
       tmp *= a;
       return tmp;
    }

    // ------------- I/O to/from streams -------------

    template< class char_t, class traits_t, class Coords >
    inline
    std::basic_ostream<char_t,traits_t> &
    operator << ( std::basic_ostream<char_t,traits_t> & os
                  , LorentzVector<Coords> const & v
       )
    {
       if( !os )  return os;

       typename Coords::Scalar a, b, c, d;
       v.GetCoordinates(a, b, c, d);

       if( detail::get_manip( os, detail::bitforbit ) )  {
        detail::set_manip( os, detail::bitforbit, '\00' );
        // TODO: call MF's bitwise-accurate functions on each of a, b, c, d
       }
       else  {
          os << detail::get_manip( os, detail::open  ) << a
             << detail::get_manip( os, detail::sep   ) << b
             << detail::get_manip( os, detail::sep   ) << c
             << detail::get_manip( os, detail::sep   ) << d
             << detail::get_manip( os, detail::close );
       }

       return os;

    }  // op<< <>()


     template< class char_t, class traits_t, class Coords >
     inline
     std::basic_istream<char_t,traits_t> &
     operator >> ( std::basic_istream<char_t,traits_t> & is
                   , LorentzVector<Coords> & v
        )
     {
        if( !is )  return is;

        typename Coords::Scalar a, b, c, d;

        if( detail::get_manip( is, detail::bitforbit ) )  {
           detail::set_manip( is, detail::bitforbit, '\00' );
           // TODO: call MF's bitwise-accurate functions on each of a, b, c
        }
        else  {
           detail::require_delim( is, detail::open  );  is >> a;
           detail::require_delim( is, detail::sep   );  is >> b;
           detail::require_delim( is, detail::sep   );  is >> c;
           detail::require_delim( is, detail::sep   );  is >> d;
           detail::require_delim( is, detail::close );
        }

        if( is )
           v.SetCoordinates(a, b, c, d);
        return is;

     }  // op>> <>()



  } // end namespace Experimental

} // end namespace ROOT

#include <sstream>
namespace cling
{
template<typename CoordSystem>
std::string printValue(const ROOT::Experimental::LorentzVector<CoordSystem> *v)
{
   std::stringstream s;
   s << *v;
   return s.str();
}

} // end namespace cling

#endif

//#include "SYCLMath/GenVector/LorentzVectorOperations.h"
