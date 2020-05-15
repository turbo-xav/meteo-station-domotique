import { Injectable } from '@angular/core';
import { EnvironmentDetail } from '../interfaces/environmentDetail';
@Injectable({
  providedIn: 'root'
})
export class EnvironmentService {

  constructor() { }

  public getEnvironnent(): EnvironmentDetail {
    const environmentDetail =  !!localStorage.getItem('env') ? JSON.parse(localStorage.getItem('env')): null ;
    if(!environmentDetail || !environmentDetail.environnement){
      return null;
    }
    return new EnvironmentDetail( environmentDetail.environment.apis.thingerio, environmentDetail.environment.apis.forecast);
    
  }

  public setEnvironnent(thingerio, forecast) {
   const environmentDetail = new EnvironmentDetail( thingerio, forecast);
   localStorage.setItem('env', JSON.stringify(environmentDetail));
  }

  public destroyEnv(){
    localStorage.removeItem('env');
  }
}
