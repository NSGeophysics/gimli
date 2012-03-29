/***************************************************************************
 *   Copyright (C) 2012 by the resistivity.net development team            *
 *   Carsten R�cker carsten@resistivity.net                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "gravimetry.h"
#include <cmath>

namespace GIMLI {

GravimetryModelling::GravimetryModelling( Mesh & mesh, DataContainer & dataContainer, bool verbose ){
}

RVector GravimetryModelling::createDefaultStartModel( ){
    RVector ret;
    THROW_TO_IMPL
    return ret;
}

RVector GravimetryModelling::response( const RVector & model ){
    RVector ret;
    THROW_TO_IMPL
    return ret;
}

void GravimetryModelling::createJacobian( const RVector & model ){
    THROW_TO_IMPL
}

void GravimetryModelling::initJacobian( ){
    THROW_TO_IMPL
}


double lineIntegraldGdz( const RVector3 & p1, const RVector3 & p2 ){
    double x1 = p1[ 0 ], z1 = p1[ 1 ];
    double x2 = p2[ 0 ], z2 = p2[ 1 ];
        
    if ( ( ::fabs( x1 ) < TOLERANCE ) && ( ::fabs( z1 ) < TOLERANCE ) ) return 0.0;
    if ( ( ::fabs( x2 ) < TOLERANCE ) && ( ::fabs( z2 ) < TOLERANCE ) ) return 0.0;
        
    double theta1 = ::atan2( z1, x1 );
    double theta2 = ::atan2( z2, x2 );

    double r1 = ::sqrt( x1*x1 + z1*z1 );
    double r2 = ::sqrt( x2*x2 + z2*z2 );
    
    // z-component of gravitational field    
    double Z = 0.0;
    // x-component of gravitational field    
    // double X = 0.0;
    
    if ( sign( z1 ) != sign( z2 ) ){
       
        if ( ( x1*z2 < x2*z1 ) && ( z2 >=0.0 ) ) {
            theta1 += PI2;
        } else if( ( x1*z2 > x2*z1 ) && ( z1 >=0.0 ) ){
            theta2 += PI2;
        } else if( ::fabs( x1*z2 - x2*z1 ) < TOLERANCE ){
            // X = Z = 0
            return 0.0;
        }
    }            

    if ( ::fabs( x1 - x2 ) < TOLERANCE ){ // case 3
        Z = x1 * ::log( r2 / r1 );
        // X = -x1 * ( theta1 - theta2 );
    } else { // default
    
        double B = ( z2 - z1 ) / ( x2 - x1 );
        double A = ( ( x2 - x1 ) * ( x1 * z2 - x2 * z1 ) ) / ( ( x2 - x1 )*( x2 - x1 ) + ( z2 - z1 )*( z2 - z1 ) );

        Z = A * ( ( theta1 - theta2 ) + B * ::log( r2 / r1 ) );
        // X = A * ( -( theta1 - theta2 ) B + ::log( r2 / r1 ) );
    }
        
    return Z;
}

RVector calcGBounds( const std::vector< RVector3 > & pos, const Mesh & mesh, const RVector & model ){
    /*! Ensure neighbourInfos() */
    RMatrix Jacobian( pos.size(), mesh.cellCount() );
    
    Jacobian *= 0.;
    
    for ( uint i = 0; i < pos.size(); i ++ ){
        for ( std::vector< Boundary * >::const_iterator it = mesh.boundaries().begin(); it != mesh.boundaries().end(); it ++ ){
            Boundary *b = *it;
            double Z = lineIntegraldGdz( b->node( 0 ).pos() - pos[ i ], b->node( 1 ).pos() - pos[ i ] );
            
            if ( b->leftCell() ) Jacobian[ i ][ b->leftCell()->id() ] = Jacobian[ i ][ b->leftCell()->id() ] - Z;
            if ( b->rightCell() ) Jacobian[ i ][ b->rightCell()->id() ] = Jacobian[ i ][ b->rightCell()->id() ] + Z;
        }
    }
                
    return Jacobian * model * 2.0 * 6.67384e-11 * 1e5;
}


RVector calcGCells( const std::vector< RVector3 > & pos, const Mesh & mesh, const RVector & model ){
    /*! Ensure neighbourInfos() */
    RMatrix Jacobian( pos.size(), mesh.cellCount() );
    
    Jacobian *= 0.;
    
    for ( uint i = 0; i < pos.size(); i ++ ){
        for ( std::vector< Cell * >::const_iterator it = mesh.cells().begin(); it != mesh.cells().end(); it ++ ){
            Cell *c = *it;
            double Z = 0.;
            for ( uint j = 0; j < c->nodeCount(); j ++ ){
                Z += lineIntegraldGdz( c->node( j ).pos() - pos[ i ], c->node( (j+1)%c->nodeCount() ).pos() - pos[ i ] );
            }
            
            // negative Z because all cells are numbered counterclockwise
            Jacobian[ i ][ c->id() ] = -Z;
        }
    }
                
    return Jacobian * model * 2.0 * 6.67384e-11 * 1e5;
}

} // namespace GIMLI{